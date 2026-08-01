#include "MachineRequests.h"

#include "DatabaseHelpers.h"
#include "EntityHelpers.h"

using json = nlohmann::json;

json MachineRequests::List(Database &db, const json &input)
{
   EntityListDescriptor descriptor = EntityHelpers::ListMachines();
   return DatabaseHelpers::ListEntities(descriptor, db);
}

json MachineRequests::Create(Database &db, const json &input)
{
   EntityCreateDescriptor descriptor = EntityHelpers::CreateMachine(input);
   return DatabaseHelpers::InsertEntity(descriptor, input, db);
}

json MachineRequests::Delete(Database &db, const json &input)
{
   if (!input.contains("id"))
   {
      json response;
      response["status"] = "error";
      response["error"]["message"] = "Missing Id";
      return response;
   }
   return DatabaseHelpers::DeleteEntity(input["id"], "Machine", db);
}
