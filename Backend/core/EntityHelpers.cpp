#include "EntityHelpers.h"

#include "EntityValidators.h"

using json = nlohmann::json;

EntityListDescriptor EntityHelpers::ListMachines()
{
   EntityListDescriptor descriptor;
   descriptor.rootField = "machines";
   descriptor.table = "Machine";
   descriptor.selectFields = "Id, Name, Cpu, Gpu, RamGb, Motherboard";
   descriptor.selectMapper = [](sqlite3_stmt* sqlStatement, json& jsonObj)
   {
      jsonObj["cpu"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
      jsonObj["gpu"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 3));
      jsonObj["ramGb"] = sqlite3_column_int(sqlStatement, 4);
      jsonObj["motherboard"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 5));
   };
   return descriptor;
}

EntityListDescriptor EntityHelpers::ListHardwareConfigs()
{
   EntityListDescriptor descriptor;
   descriptor.rootField = "configs";
   descriptor.table = "HardwareConfiguration";
   descriptor.selectFields = "Id, MachineId, CpuFreqGhz, GpuFreqMhz, RamFreqMhz, Settings";
   descriptor.selectMapper = [](sqlite3_stmt* sqlStatement, json& jsonObj)
   {
      jsonObj["machineId"] = sqlite3_column_int(sqlStatement, 2);
      jsonObj["cpuFreqGhz"] = sqlite3_column_double(sqlStatement, 3);
      jsonObj["gpuFreqMhz"] = sqlite3_column_double(sqlStatement, 4);
      jsonObj["ramFreqMhz"] = sqlite3_column_double(sqlStatement, 5);

      const char* settingsText =
         reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 6));

      if (settingsText)
      {
         json parsed = json::parse(settingsText, nullptr, false);
         jsonObj["settings"] = parsed.is_discarded() ? json::object() : parsed;
      }
      else
         jsonObj["settings"] = json::object();
   };
   return descriptor;
}

EntityListDescriptor EntityHelpers::ListSoftwareEnvironments()
{
   EntityListDescriptor descriptor;
   descriptor.rootField = "softwareEnvironments";
   descriptor.table = "SoftwareEnvironment";
   descriptor.selectFields = "Id, Name, Os, OsVersion, DriverFamily";
   descriptor.selectMapper = [](sqlite3_stmt* sqlStatement, json& jsonObj)
   {
      jsonObj["os"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
      jsonObj["osVersion"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 3));
      jsonObj["driverFamily"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 4));
   };
   return descriptor;
}

EntityListDescriptor EntityHelpers::ListSoftwareConfigs()
{
   EntityListDescriptor descriptor;
   descriptor.rootField = "softwareConfigurations";
   descriptor.table = "SoftwareConfiguration";
   descriptor.selectFields = "Id, Name, SoftwareEnvironmentId, DriverVersion, Mode, Settings";
   descriptor.selectMapper = [](sqlite3_stmt* sqlStatement, json& jsonObj)
   {
      jsonObj["softwareEnvironmentId"] = sqlite3_column_int(sqlStatement, 2);
      jsonObj["driverVersion"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 3));
      jsonObj["mode"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 4));

      const char* settingsText =
         reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 5));

      if (settingsText)
      {
         json parsed = json::parse(settingsText, nullptr, false);
         jsonObj["settings"] = parsed.is_discarded() ? json::object() : parsed;
      }
      else
         jsonObj["settings"] = json::object();
   };
   return descriptor;
}

EntityListDescriptor EntityHelpers::ListTests()
{
   EntityListDescriptor descriptor;
   descriptor.rootField = "tests";
   descriptor.table = "Test";
   descriptor.selectFields = "Id, Name, Description, IconPath";
   descriptor.selectMapper = [](sqlite3_stmt* sqlStatement, json& jsonObj)
   {
      jsonObj["description"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
      jsonObj["iconPath"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 3));
   };
   return descriptor;
}

EntityListDescriptor EntityHelpers::ListTestConfigs()
{
   EntityListDescriptor descriptor;
   descriptor.rootField = "testConfigurations";
   descriptor.table = "TestConfiguration";
   descriptor.selectFields = "Id, Name, TestId, Settings";
   descriptor.selectMapper = [](sqlite3_stmt* sqlStatement, json& jsonObj)
   {
      jsonObj["testId"] = sqlite3_column_int(sqlStatement, 2);

      const char* settingsText =
         reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 3));

      if (settingsText)
      {
         json parsed = json::parse(settingsText, nullptr, false);
         jsonObj["settings"] = parsed.is_discarded() ? json::object() : parsed;
      }
      else
         jsonObj["settings"] = json::object();
   };
   return descriptor;
}

EntityListDescriptor EntityHelpers::ListOrigins()
{
   EntityListDescriptor descriptor;
   descriptor.rootField = "origins";
   descriptor.table = "Origin";
   // Alias OriginType as the Name column (column 1) for the generic ListEntities mapper.
   descriptor.selectFields = "Id, OriginType, RunId, ExternalId, SourceFile, CreatedAt";
   descriptor.selectMapper = [](sqlite3_stmt* stmt, json& obj)
   {
      obj["runId"] = sqlite3_column_int(stmt, 2);

      const char* externalId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
      obj["externalId"] = externalId ? externalId : "";

      const char* sourceFile = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
      obj["sourceFile"] = sourceFile ? sourceFile : "";

      const char* createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
      obj["createdAt"] = createdAt ? createdAt : "";
   };
   return descriptor;
}

EntityCreateDescriptor EntityHelpers::CreateMachine(const json &data)
{
   EntityCreateDescriptor descriptor;
   descriptor.table = "Machine";
   descriptor.insertFields = { "Name", "Cpu", "Gpu", "RamGb", "Motherboard" };
   descriptor.insertBinder = [&data](sqlite3_stmt* stmt)
   {
      sqlite3_bind_text(stmt, 1, data.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, data.value("cpu", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, data.value("gpu", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 4, data.value("ramGb", 0));
      sqlite3_bind_text(stmt, 5, data.value("motherboard", "").c_str(), -1, SQLITE_TRANSIENT);
   };
   descriptor.validator = ValidateMachine;
   return descriptor;
}

EntityCreateDescriptor EntityHelpers::CreateHardwareConfig(const nlohmann::json &data)
{
   EntityCreateDescriptor descriptor;
   descriptor.table = "HardwareConfiguration";
   descriptor.insertFields = {
      "Name", "MachineId", "CpuFreqGhz", "GpuFreqMhz", "RamFreqMhz", "Settings"
   };

   descriptor.insertBinder = [&data](sqlite3_stmt* stmt)
   {
      sqlite3_bind_text(stmt, 1, data.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 2, data.value("machineId", -1));
      sqlite3_bind_double(stmt, 3, data.value("cpuFreqGhz", 0.0));
      sqlite3_bind_double(stmt, 4, data.value("gpuFreqMhz", 0.0));
      sqlite3_bind_double(stmt, 5, data.value("ramFreqMhz", 0.0));
      sqlite3_bind_text(stmt, 6, data.value("settings", "").c_str(), -1, SQLITE_TRANSIENT);
   };

   descriptor.validator = ValidateHardwareConfiguration;
   return descriptor;
}

EntityCreateDescriptor EntityHelpers::CreateSoftwareEnvironment(const nlohmann::json &data)
{
   EntityCreateDescriptor descriptor;
   descriptor.table = "SoftwareEnvironment";
   descriptor.insertFields = {
      "Name",
      "Os",
      "OsVersion",
      "DriverFamily"
   };
   descriptor.insertBinder = [&data](sqlite3_stmt* stmt)
   {
      sqlite3_bind_text(stmt, 1, data.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, data.value("os", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, data.value("osVersion", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 4, data.value("driverFamily", "").c_str(), -1, SQLITE_TRANSIENT);
   };
   descriptor.validator = ValidateSoftwareEnvironment;
   return descriptor;
}

EntityCreateDescriptor EntityHelpers::CreateSoftwareConfig(const nlohmann::json &data)
{
   EntityCreateDescriptor descriptor;
   descriptor.table = "SoftwareConfiguration";
   descriptor.insertFields = {
      "Name",
      "SoftwareEnvironmentId",
      "DriverVersion",
      "Mode",
      "Settings"
   };

   descriptor.insertBinder = [&data](sqlite3_stmt* stmt)
   {
      sqlite3_bind_text(stmt, 1, data.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 2, data.value("softwareEnvironmentId", 0));
      sqlite3_bind_text(stmt, 3, data.value("driverVersion", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 4, data.value("mode", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 5, data.value("settings", "{}").c_str(), -1, SQLITE_TRANSIENT);
   };

   descriptor.validator = ValidateSoftwareConfiguration;
   return descriptor;
}

EntityCreateDescriptor EntityHelpers::CreateTest(const nlohmann::json &data)
{
   EntityCreateDescriptor descriptor;
   descriptor.table = "Test";
   descriptor.insertFields = {
      "Name",
      "Description",
      "IconPath"
   };

   descriptor.insertBinder = [&data](sqlite3_stmt* stmt)
   {
      sqlite3_bind_text(stmt, 1, data.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, data.value("description", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, data.value("iconPath", "").c_str(), -1, SQLITE_TRANSIENT);
   };

   descriptor.validator = ValidateTest;
   return descriptor;
}

EntityCreateDescriptor EntityHelpers::CreateTestConfig(const nlohmann::json &data)
{
   EntityCreateDescriptor descriptor;
   descriptor.table = "TestConfiguration";
   descriptor.insertFields = {
      "Name",
      "TestId",
      "Settings"
   };

   descriptor.insertBinder = [&data](sqlite3_stmt* stmt)
   {
      sqlite3_bind_text(stmt, 1, data.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 2, data.value("testId", 0));
      sqlite3_bind_text(stmt, 3, data.value("settings", "{}").c_str(), -1, SQLITE_TRANSIENT);
   };
   descriptor.validator = ValidateTestConfiguration;
   return descriptor;
}
