#include "SoftwareConfigRequestHandler.h"

#include "DatabaseHelpers.h"
#include "EntityHelpers.h"

using json = nlohmann::json;

json SoftwareConfigRequestHandler::List(Database &db, const json &input)
{
   EntityListDescriptor descriptor = EntityHelpers::ListSoftwareConfigs();
   return DatabaseHelpers::ListEntities(descriptor, db);
}

json SoftwareConfigRequestHandler::Create(Database &db, const json &input)
{
   EntityCreateDescriptor descriptor = EntityHelpers::CreateSoftwareConfig(input);
   return DatabaseHelpers::InsertEntity(descriptor, input, db);
}

json SoftwareConfigRequestHandler::Delete(Database &db, const json &input)
{
   if (!input.contains("id"))
   {
      return CreateInvalidIdResponse();
   }
   return DatabaseHelpers::DeleteEntity(input["id"], "SoftwareConfiguration", db);
}
