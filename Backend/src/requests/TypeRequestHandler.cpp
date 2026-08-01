#include "TypeRequestHandler.h"

nlohmann::json TypeRequestHandler::CreateInvalidIdResponse() const
{
   nlohmann::json response;
   response["status"] = "error";
   response["error"]["message"] = "Missing Id";
   return response;
}
