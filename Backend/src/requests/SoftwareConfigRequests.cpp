#include "SoftwareConfigRequests.h"

#include "DatabaseHelpers.h"
#include "EntityHelpers.h"

using json = nlohmann::json;

json SoftwareConfigRequests::List(Database &db, const json &input)
{
   EntityListDescriptor descriptor = EntityHelpers::ListSoftwareConfigs();
   return DatabaseHelpers::ListEntities(descriptor, db);
}

json SoftwareConfigRequests::Create(Database &db, const json &input)
{
   EntityCreateDescriptor descriptor = EntityHelpers::CreateSoftwareConfig(input);
   return DatabaseHelpers::InsertEntity(descriptor, input, db);
}

json SoftwareConfigRequests::Delete(Database &db, const json &input)
{
   if (!input.contains("id"))
   {
      json response;
      response["status"] = "error";
      response["error"]["message"] = "Missing Id";
      return response;
   }
   return DatabaseHelpers::DeleteEntity(input["id"], "SoftwareConfiguration", db);
}
