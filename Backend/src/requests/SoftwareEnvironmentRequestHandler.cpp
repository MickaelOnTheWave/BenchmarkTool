#include "SoftwareEnvironmentRequestHandler.h"

#include "DatabaseHelpers.h"
#include "EntityHelpers.h"

using json = nlohmann::json;

json SoftwareEnvironmentRequestHandler::List(Database &db, const json &input)
{
   EntityListDescriptor descriptor = EntityHelpers::ListSoftwareEnvironments();
   return DatabaseHelpers::ListEntities(descriptor, db);
}

json SoftwareEnvironmentRequestHandler::Create(Database &db, const json &input)
{
   EntityCreateDescriptor descriptor = EntityHelpers::CreateSoftwareEnvironment(input);
   return DatabaseHelpers::InsertEntity(descriptor, input, db);
}

json SoftwareEnvironmentRequestHandler::Delete(Database &db, const json &input)
{
   if (!input.contains("id"))
   {
      return CreateInvalidIdResponse();
   }
   return DatabaseHelpers::DeleteEntity(input["id"], "SoftwareEnvironment", db);
}
