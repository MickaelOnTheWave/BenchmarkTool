#include "TestConfigRequestHandler.h"

#include "DatabaseHelpers.h"
#include "EntityHelpers.h"

using json = nlohmann::json;

json TestConfigRequestHandler::List(Database &db, const json &input)
{
   EntityListDescriptor descriptor = EntityHelpers::ListTestConfigs();
   return DatabaseHelpers::ListEntities(descriptor, db);
}

json TestConfigRequestHandler::Create(Database &db, const json &input)
{
   EntityCreateDescriptor descriptor = EntityHelpers::CreateTestConfig(input);
   return DatabaseHelpers::InsertEntity(descriptor, input, db);
}

json TestConfigRequestHandler::Delete(Database &db, const json &input)
{
   if (!input.contains("id"))
   {
      return CreateInvalidIdResponse();
   }
   return DatabaseHelpers::DeleteEntity(input["id"], "TestConfiguration", db);
}
