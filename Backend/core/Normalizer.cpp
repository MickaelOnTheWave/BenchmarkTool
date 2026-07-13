#include "Normalizer.h"

#include "Database.h"
#include "Utils.h"

using json = nlohmann::json;

Normalizer::Normalizer(Database& _db)
   : db(_db)
{
}

int Normalizer::ResolvePlanId(const json& subPlan)
{
   if (subPlan.is_object() && subPlan.value("action", "") == "reuse")
      return subPlan.value("id", 0);
   return 0;
}

json Normalizer::Normalize(const json& importedData)
{
   json executionPlan;

   executionPlan["machine"] = NormalizeMachine(importedData);

   const int machineId = ResolvePlanId(executionPlan["machine"]);
   if (machineId > 0)
      executionPlan["hardwareconfig"] = NormalizeHardwareConfig(machineId, importedData);
   else
   {
      json hardwareConfig;
      SetCreateHardwareConfig(importedData.value("machine", json::object()), hardwareConfig);
      executionPlan["hardwareconfig"] = hardwareConfig;
   }

   executionPlan["softwareenvironment"] = NormalizeSoftwareEnvironment(importedData);

   const int environmentId = ResolvePlanId(executionPlan["softwareenvironment"]);
   if (environmentId > 0)
      executionPlan["softwareconfig"] = NormalizeSoftwareConfig(environmentId, importedData);
   else
   {
      json softwareConfig;
      SetCreateSoftwareConfig(importedData.value("softwareenvironment", json::object()), softwareConfig);
      executionPlan["softwareconfig"] = softwareConfig;
   }

   executionPlan["test"] = NormalizeTest(importedData);

   const int testId = ResolvePlanId(executionPlan["test"]);
   if (testId > 0)
      executionPlan["testconfig"] = NormalizeTestConfig(testId, importedData);
   else
   {
      json testConfig;
      SetCreateTestConfig(importedData.value("test", json::object()), testConfig);
      executionPlan["testconfig"] = testConfig;
   }

   executionPlan["benchmarkrun"] = NormalizeBenchmarkRun(executionPlan, importedData);

   return executionPlan;
}

json Normalizer::NormalizeMachine(const json& importedData)
{
   json result;
   const json& machineData = importedData["machine"];

   const int machineId = FindMatchingMachine(machineData);

   if (machineId > 0)
   {
      result["action"] = "reuse";
      result["id"] = machineId;
   }
   else
   {
      result["action"] = "create";
      result["data"]["name"] = "";
      result["data"]["cpu"] = Utils::GetNested(machineData, {"cpu", "name"});
      result["data"]["gpu"] = Utils::GetNested(machineData, {"gpu", "name"});
      result["data"]["motherboard"] = Utils::GetNested(machineData, {"motherboard", "model"});
      result["data"]["ramGb"] = machineData.value("ram", 0);
   }

   return result;
}

json Normalizer::NormalizeHardwareConfig(const int machineId, const json& importedData)
{
   json result;
   const json& machineData = importedData["machine"];

   const int hardwareConfigId = FindMatchingHardwareConfig(machineId, machineData);
   if (hardwareConfigId > 0)
   {
      result["action"] = "reuse";
      result["id"] = hardwareConfigId;
   }
   else
      SetCreateHardwareConfig(machineData, result);
   return result;
}

json Normalizer::NormalizeSoftwareEnvironment(const json& importedData)
{
   json result;
   const json envData = importedData.value("softwareenvironment", json::object());

   const int environmentId = FindMatchingSoftwareEnvironment(envData);
   if (environmentId > 0)
   {
      result["action"] = "reuse";
      result["id"] = environmentId;
   }
   else
      SetCreateSoftwareEnvironment(envData, result);
   return result;
}

json Normalizer::NormalizeSoftwareConfig(const int softwareEnvironmentId, const json& importedData)
{
   json result;
   const json envData = importedData.value("softwareenvironment", json::object());

   const int configId = FindMatchingSoftwareConfig(softwareEnvironmentId, envData);
   if (configId > 0)
   {
      result["action"] = "reuse";
      result["id"] = configId;
   }
   else
      SetCreateSoftwareConfig(envData, result);
   return result;
}

json Normalizer::NormalizeTest(const json& importedData)
{
   json result;
   const json testData = importedData.value("test", json::object());

   const int testId = FindMatchingTest(testData);
   if (testId > 0)
   {
      result["action"] = "reuse";
      result["id"] = testId;
   }
   else
      SetCreateTest(testData, result);
   return result;
}

json Normalizer::NormalizeTestConfig(const int testId, const json& importedData)
{
   json result;
   const json testData = importedData.value("test", json::object());

   const int configId = FindMatchingTestConfig(testId, BuildTestConfigSettings(testData));
   if (configId > 0)
   {
      result["action"] = "reuse";
      result["id"] = configId;
   }
   else
      SetCreateTestConfig(testData, result);
   return result;
}

json Normalizer::NormalizeBenchmarkRun(const json& executionPlan, const json& importedData)
{
   json result;
   const json runData = importedData.value("benchmarkRun", json::object());

   // A benchmark run is always unique (it references a specific point in time),
   // so it is always created rather than reused. It references the resolved
   // ids of the other entities where those are reused; 0 means "will be created".
   result["action"] = "create";
   result["data"]["machineId"] = ResolvePlanId(executionPlan.value("machine", json::object()));
   result["data"]["hardwareConfigurationId"] = ResolvePlanId(executionPlan.value("hardwareconfig", json::object()));
   result["data"]["softwareEnvironmentId"] = ResolvePlanId(executionPlan.value("softwareenvironment", json::object()));
   result["data"]["softwareConfigurationId"] = ResolvePlanId(executionPlan.value("softwareconfig", json::object()));
   result["data"]["testId"] = ResolvePlanId(executionPlan.value("test", json::object()));
   result["data"]["testConfigurationId"] = ResolvePlanId(executionPlan.value("testconfig", json::object()));

   result["data"]["timestamp"] = runData.value("time", "");

   json resultData = runData.value("result", json::object());
   result["data"]["result"]["score"] = resultData.value("score", 0);
   result["data"]["result"]["avgFps"] = resultData.value("avgFps", 0);
   result["data"]["result"]["minFps"] = resultData.value("minFps", 0);
   result["data"]["result"]["maxFps"] = resultData.value("maxFps", 0);

   result["data"]["origin"]["externalId"] = Utils::GetNested(runData, {"origin", "runExternalId"});

   return result;
}

int Normalizer::FindMatchingMachine(const json& machineData)
{
   const std::string cpu = machineData.contains("cpu") ? machineData["cpu"].value("name", "") : "";
   const std::string gpu = machineData.contains("gpu") ? machineData["gpu"].value("name", "") : "";
   const std::string motherboard = machineData.contains("motherboard") ? machineData["motherboard"].value("model", "") : "";
   const int ram = machineData.value("ram", 0);

   const std::string query =
      "SELECT Id FROM Machine "
      "WHERE Cpu = ? AND Gpu = ? AND RamGb = ? AND Motherboard = ?;";

   sqlite3_stmt* stmt = nullptr;

   if (sqlite3_prepare_v2(db.GetHandle(), query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
      return 0;

   sqlite3_bind_text(stmt, 1, cpu.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 2, gpu.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_int(stmt, 3, ram);
   sqlite3_bind_text(stmt, 4, motherboard.c_str(), -1, SQLITE_TRANSIENT);

   int resultId = 0;

   if (sqlite3_step(stmt) == SQLITE_ROW)
      resultId = sqlite3_column_int(stmt, 0);

   sqlite3_finalize(stmt);

   return resultId;
}

int Normalizer::FindMatchingHardwareConfig(const int machineId, const json& machineData)
{
   // The importer emits frequencies as strings; the columns have REAL affinity
   // so SQLite coerces the bound text for comparison.
   const std::string cpuFreq = Utils::GetNested(machineData, {"cpu", "frequency"});
   const std::string gpuFreq = Utils::GetNested(machineData, {"gpu", "frequency"});

   const std::string query =
      "SELECT Id FROM HardwareConfiguration "
      "WHERE MachineId = ? AND CpuFreqGhz = ? AND GpuFreqMhz = ?;";

   sqlite3_stmt* stmt = nullptr;

   if (sqlite3_prepare_v2(db.GetHandle(), query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
      return 0;

   sqlite3_bind_int(stmt, 1, machineId);
   sqlite3_bind_text(stmt, 2, cpuFreq.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 3, gpuFreq.c_str(), -1, SQLITE_TRANSIENT);

   int resultId = 0;

   if (sqlite3_step(stmt) == SQLITE_ROW)
      resultId = sqlite3_column_int(stmt, 0);

   sqlite3_finalize(stmt);

   return resultId;
}

int Normalizer::FindMatchingSoftwareEnvironment(const json& envData)
{
   const std::string os = envData.value("os", "");
   const std::string osVersion = "";
   const std::string driverFamily = "";

   const std::string query =
      "SELECT Id FROM SoftwareEnvironment "
      "WHERE Os = ? AND OsVersion = ? AND DriverFamily = ?;";

   sqlite3_stmt* stmt = nullptr;

   if (sqlite3_prepare_v2(db.GetHandle(), query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
      return 0;

   sqlite3_bind_text(stmt, 1, os.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 2, osVersion.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 3, driverFamily.c_str(), -1, SQLITE_TRANSIENT);

   int resultId = 0;

   if (sqlite3_step(stmt) == SQLITE_ROW)
      resultId = sqlite3_column_int(stmt, 0);

   sqlite3_finalize(stmt);

   return resultId;
}

int Normalizer::FindMatchingSoftwareConfig(const int softwareEnvironmentId, const json& envData)
{
   const std::string driverVersion = envData.value("driverversion", "");
   const std::string mode = "";

   const std::string query =
      "SELECT Id FROM SoftwareConfiguration "
      "WHERE SoftwareEnvironmentId = ? AND DriverVersion = ? AND Mode = ?;";

   sqlite3_stmt* stmt = nullptr;

   if (sqlite3_prepare_v2(db.GetHandle(), query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
      return 0;

   sqlite3_bind_int(stmt, 1, softwareEnvironmentId);
   sqlite3_bind_text(stmt, 2, driverVersion.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 3, mode.c_str(), -1, SQLITE_TRANSIENT);

   int resultId = 0;

   if (sqlite3_step(stmt) == SQLITE_ROW)
      resultId = sqlite3_column_int(stmt, 0);

   sqlite3_finalize(stmt);

   return resultId;
}

int Normalizer::FindMatchingTest(const json& testData)
{
   const std::string name = testData.value("name", "");

   const std::string query = "SELECT Id FROM Test WHERE Name = ?;";

   sqlite3_stmt* stmt = nullptr;

   if (sqlite3_prepare_v2(db.GetHandle(), query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
      return 0;

   sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

   int resultId = 0;

   if (sqlite3_step(stmt) == SQLITE_ROW)
      resultId = sqlite3_column_int(stmt, 0);

   sqlite3_finalize(stmt);

   return resultId;
}

int Normalizer::FindMatchingTestConfig(const int testId, const std::string& settings)
{
   const std::string query =
      "SELECT Id FROM TestConfiguration WHERE TestId = ? AND Settings = ?;";

   sqlite3_stmt* stmt = nullptr;

   if (sqlite3_prepare_v2(db.GetHandle(), query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
      return 0;

   sqlite3_bind_int(stmt, 1, testId);
   sqlite3_bind_text(stmt, 2, settings.c_str(), -1, SQLITE_TRANSIENT);

   int resultId = 0;

   if (sqlite3_step(stmt) == SQLITE_ROW)
      resultId = sqlite3_column_int(stmt, 0);

   sqlite3_finalize(stmt);

   return resultId;
}

void Normalizer::SetCreateHardwareConfig(const json& machineData, json& result)
{
   result["action"] = "create";
   result["data"]["name"] = "";
   result["data"]["cpuFreqGhz"] = Utils::GetNested(machineData, {"cpu", "frequency"});
   result["data"]["gpuFreqMhz"] = Utils::GetNested(machineData, {"gpu", "frequency"});
   result["data"]["ramFreqMhz"] = "";
   result["data"]["settings"] = "";
}

void Normalizer::SetCreateSoftwareEnvironment(const json& envData, json& result)
{
   result["action"] = "create";
   result["data"]["name"] = "";
   result["data"]["os"] = envData.value("os", "");
   result["data"]["osVersion"] = "";
   result["data"]["driverFamily"] = "";
}

void Normalizer::SetCreateSoftwareConfig(const json& envData, json& result)
{
   result["action"] = "create";
   result["data"]["name"] = "";
   result["data"]["driverVersion"] = envData.value("driverversion", "");
   result["data"]["mode"] = "";

   json settings;
   settings["biosVersion"] = Utils::GetNested(envData, {"bios", "version"});
   settings["biosDate"] = Utils::GetNested(envData, {"bios", "date"});
   result["data"]["settings"] = settings.dump();
}

void Normalizer::SetCreateTest(const json& testData, json& result)
{
   result["action"] = "create";
   result["data"]["name"] = testData.value("name", "");
   result["data"]["description"] = "";
   result["data"]["iconPath"] = "";
}

void Normalizer::SetCreateTestConfig(const json& testData, json& result)
{
   result["action"] = "create";
   result["data"]["name"] = "";
   result["data"]["settings"] = BuildTestConfigSettings(testData);
}

std::string Normalizer::BuildTestConfigSettings(const json& testData)
{
   json settings;
   settings["usedGpu"] = testData.value("usedGpu", "");
   settings["textureFilterMode"] = testData.value("textureFilterMode", "");
   settings["resolution"] = testData.value("resolution", "");
   return settings.dump();
}
