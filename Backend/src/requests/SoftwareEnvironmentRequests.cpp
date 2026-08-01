#include "SoftwareEnvironmentRequests.h"

#include "DatabaseHelpers.h"
#include "EntityHelpers.h"

using json = nlohmann::json;

json SoftwareEnvironmentRequests::List(Database &db, const json &input)
{
   EntityListDescriptor descriptor = EntityHelpers::ListSoftwareEnvironments();
   return DatabaseHelpers::ListEntities(descriptor, db);
}

json SoftwareEnvironmentRequests::Create(Database &db, const json &input)
{
   EntityCreateDescriptor descriptor = EntityHelpers::CreateSoftwareEnvironment(input);
   return DatabaseHelpers::InsertEntity(descriptor, input, db);
}

json SoftwareEnvironmentRequests::Delete(Database &db, const json &input)
{
   if (!input.contains("id"))
   {
      json response;
      response["status"] = "error";
      response["error"]["message"] = "Missing Id";
      return response;
   }
   return DatabaseHelpers::DeleteEntity(input["id"], "SoftwareEnvironment", db);
}
