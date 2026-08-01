#include "AddFullMachineRequest.h"

#include <sqlite3.h>

#include "MachineRequests.h"
#include "DatabaseHelpers.h"
#include "EntityHelpers.h"

using json = nlohmann::json;

json AddFullMachineRequest::CreateJsonResponse(Database &db, const json &input)
{
   json output;
   // Start transaction
   char* errMsg = nullptr;
   if (sqlite3_exec(db.GetHandle(), "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg) != SQLITE_OK)
   {
      output["status"] = "error";
      output["error"]["type"] = "BEGIN TRANSACTION;";
      output["error"]["message"] = errMsg ? errMsg : "Failed to start transaction";
      sqlite3_free(errMsg);
      return output;
   }


   json jsonData = input.value("machine", json::object());
   json result = MachineRequests::Create(db, jsonData);
//   const EntityDescriptor machine = EntityHelpers::CreateMachine(jsonData);
//   json result = DatabaseHelpers::InsertEntity(machine, jsonData, db);
   if (result["status"] == "error")
      return result;
   const int machineId = result["id"];

   jsonData = input.value("hardwareConfig", json::object());
   jsonData["machineId"] = machineId;
   const EntityCreateDescriptor hardwareConfig = EntityHelpers::CreateHardwareConfig(jsonData);
   result = DatabaseHelpers::InsertEntity(hardwareConfig, jsonData, db);
   if (result["status"] == "error")
      return result;
   const int hwConfigId = result["id"];

   jsonData = input.value("softwareEnvironment", json::object());
   const EntityCreateDescriptor softwareEnv = EntityHelpers::CreateSoftwareEnvironment(jsonData);
   result = DatabaseHelpers::InsertEntity(softwareEnv, jsonData, db);
   if (result["status"] == "error")
      return result;
   const int swEnvId = result["id"];

   jsonData = input.value("softwareConfig", json::object());
   jsonData["softwareEnvironmentId"] = result["id"];
   const EntityCreateDescriptor softwareConfig = EntityHelpers::CreateSoftwareConfig(jsonData);
   result = DatabaseHelpers::InsertEntity(softwareConfig, jsonData, db);
   if (result["status"] == "error")
      return result;
   const int swConfigId = result["id"];

   output["status"] = "ok";
   output["machineId"] = machineId;
   output["hardwareConfigId"] = hwConfigId;
   output["softwareEnvironmentId"] = swEnvId;
   output["softwareConfigId"] = swConfigId;
   return output;
}
