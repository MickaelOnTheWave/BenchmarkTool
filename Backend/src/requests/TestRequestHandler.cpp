#include "TestRequestHandler.h"

#include "DatabaseHelpers.h"
#include "EntityHelpers.h"

using json = nlohmann::json;

json TestRequestHandler::List(Database &db, const json &input)
{
   EntityListDescriptor descriptor = EntityHelpers::ListTests();
   return DatabaseHelpers::ListEntities(descriptor, db);
}

json TestRequestHandler::Create(Database &db, const json &input)
{
   EntityCreateDescriptor descriptor = EntityHelpers::CreateTest(input);
   return DatabaseHelpers::InsertEntity(descriptor, input, db);
}

json TestRequestHandler::Delete(Database &db, const json &input)
{
   if (!input.contains("id"))
   {
      return CreateInvalidIdResponse();
   }
   return DatabaseHelpers::DeleteEntity(input["id"], "Test", db);
}
