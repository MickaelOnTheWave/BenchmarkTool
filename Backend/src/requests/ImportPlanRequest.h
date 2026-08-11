#ifndef IMPORTPLANREQUEST_H
#define IMPORTPLANREQUEST_H

#include <nlohmann/json.hpp>
#include "Database.h"

class TypeRequestHandler;

class ImportPlanRequest
{
public:
   static nlohmann::json Execute(Database& db, const nlohmann::json &input);

private:
   static int ResolveEntity(Database& db, nlohmann::json& response, const nlohmann::json& input,
                           const std::string& planKey, TypeRequestHandler& handler,
                           const std::string& parentFkField = "", int parentId = 0);
};

#endif // IMPORTPLANREQUEST_H
