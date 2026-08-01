#include "TestConfigRequests.h"

#include "DatabaseHelpers.h"
#include "EntityHelpers.h"

using json = nlohmann::json;

json TestConfigRequests::List(Database &db, const json &input)
{
   EntityListDescriptor descriptor = EntityHelpers::ListTestConfigs();
   return DatabaseHelpers::ListEntities(descriptor, db);
}

json TestConfigRequests::Create(Database &db, const json &input)
{
   EntityCreateDescriptor descriptor = EntityHelpers::CreateTestConfig(input);
   return DatabaseHelpers::InsertEntity(descriptor, input, db);
}

json TestConfigRequests::Delete(Database &db, const json &input)
{
   if (!input.contains("id"))
   {
      json response;
      response["status"] = "error";
      response["error"]["message"] = "Missing Id";
      return response;
   }
   return DatabaseHelpers::DeleteEntity(input["id"], "TestConfiguration", db);
}
