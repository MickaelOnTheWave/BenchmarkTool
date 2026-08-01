#include "TestRequests.h"

#include "DatabaseHelpers.h"
#include "EntityHelpers.h"

using json = nlohmann::json;

json TestRequests::List(Database &db, const json &input)
{
   EntityListDescriptor descriptor = EntityHelpers::ListTests();
   return DatabaseHelpers::ListEntities(descriptor, db);
}

json TestRequests::Create(Database &db, const json &input)
{
   EntityCreateDescriptor descriptor = EntityHelpers::CreateTest(input);
   return DatabaseHelpers::InsertEntity(descriptor, input, db);
}

json TestRequests::Delete(Database &db, const json &input)
{
   if (!input.contains("id"))
   {
      json response;
      response["status"] = "error";
      response["error"]["message"] = "Missing Id";
      return response;
   }
   return DatabaseHelpers::DeleteEntity(input["id"], "Test", db);
}
