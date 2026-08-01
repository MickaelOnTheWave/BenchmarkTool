#include "HardwareConfigRequestHandler.h"

#include "DatabaseHelpers.h"
#include "EntityHelpers.h"

using json = nlohmann::json;

json HardwareConfigRequestHandler::List(Database &db, const json &input)
{
   EntityListDescriptor descriptor = EntityHelpers::ListHardwareConfigs();
   return DatabaseHelpers::ListEntities(descriptor, db);
}

json HardwareConfigRequestHandler::Create(Database &db, const json &input)
{
   EntityCreateDescriptor descriptor = EntityHelpers::CreateHardwareConfig(input);
   return DatabaseHelpers::InsertEntity(descriptor, input, db);
}

json HardwareConfigRequestHandler::Delete(Database &db, const json &input)
{
   if (!input.contains("id"))
   {
      return CreateInvalidIdResponse();
   }
   return DatabaseHelpers::DeleteEntity(input["id"], "HardwareConfiguration", db);
}
