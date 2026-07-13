#include "Normalizer.h"

#include "Database.h"
#include "Utils.h"

using json = nlohmann::json;

Normalizer::Normalizer(Database& _db)
   : db(_db)
{
}

json Normalizer::Normalize(const json& importedData)
{
   json executionPlan;

   executionPlan["machine"] = NormalizeMachine(importedData);

   if (executionPlan["machine"]["action"] == "reuse")
   {
      const int machineId = executionPlan["machine"]["action"]["id"];
      executionPlan["hardwareconfig"] = NormalizeHardwareConfig(machineId, importedData);
   }
   else
   {
      json hardwareConfig;
      SetCreateHardwareConfig(importedData["machine"], hardwareConfig);
      executionPlan["hardwareconfig"] = hardwareConfig;
   }

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
      result["data"]["cpu"] = Utils::GetNested(machineData, {"cpu", "name"});
      result["data"]["gpu"] = Utils::GetNested(machineData, {"gpu", "name"});
      result["data"]["motherboard"] = Utils::GetNested(machineData, {"motherboard", "model"});
      result["data"]["ram"] = machineData["ram"];
   }

   return result;
}

nlohmann::json Normalizer::NormalizeHardwareConfig(const int machineId, const nlohmann::json& importedData)
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

int Normalizer::FindMatchingHardwareConfig(const int machineId, const nlohmann::json& machineData)
{
   const int cpuFreq = machineData["cpu"].value("frequency", 0);
   const int gpuFreq = machineData["gpu"].value("frequency", 0);

   const std::string query =
      "SELECT Id FROM HardwareConfiguration "
      "WHERE MachineId = ? AND CpuFreqGhz = ? AND GpuFreqMhz = ?;";

   sqlite3_stmt* stmt = nullptr;

   if (sqlite3_prepare_v2(db.GetHandle(), query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
      return 0;

   sqlite3_bind_int(stmt, 1, machineId);
   sqlite3_bind_int(stmt, 2, cpuFreq);
   sqlite3_bind_int(stmt, 3, gpuFreq);

   int resultId = 0;

   if (sqlite3_step(stmt) == SQLITE_ROW)
      resultId = sqlite3_column_int(stmt, 0);

   sqlite3_finalize(stmt);

   return resultId;
}

void Normalizer::SetCreateHardwareConfig(const nlohmann::json& machineData, nlohmann::json& result)
{
   result["action"] = "create";
   result["data"]["cpu-frequency"] = Utils::GetNested(machineData, {"cpu", "frequency"});
   result["data"]["gpu-frequency"] = Utils::GetNested(machineData, {"gpu", "frequency"});
   result["data"]["ram-frequency"] = "";
   result["data"]["settings"] = "";
}
