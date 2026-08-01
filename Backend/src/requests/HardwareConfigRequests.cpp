#include "HardwareConfigRequests.h"

#include "DatabaseHelpers.h"
#include "EntityHelpers.h"

using json = nlohmann::json;

json HardwareConfigRequests::List(Database &db, const json &input)
{
   EntityListDescriptor descriptor = EntityHelpers::ListHardwareConfigs();
   return DatabaseHelpers::ListEntities(descriptor, db);
}

json HardwareConfigRequests::Create(Database &db, const json &input)
{
   EntityCreateDescriptor descriptor = EntityHelpers::CreateHardwareConfig(input);
   return DatabaseHelpers::InsertEntity(descriptor, input, db);
}

json HardwareConfigRequests::Delete(Database &db, const json &input)
{
   if (!input.contains("id"))
   {
      json response;
      response["status"] = "error";
      response["error"]["message"] = "Missing Id";
      return response;
   }
   return DatabaseHelpers::DeleteEntity(input["id"], "HardwareConfiguration", db);
}
