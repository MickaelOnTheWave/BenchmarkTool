#ifndef TYPEREQUESTHANDLER_H
#define TYPEREQUESTHANDLER_H

#include <nlohmann/json.hpp>

#include "Database.h"

class TypeRequestHandler
{
public:
   virtual nlohmann::json List(Database& db, const nlohmann::json &input) = 0;
   virtual nlohmann::json Create(Database& db, const nlohmann::json &input) = 0;
   virtual nlohmann::json Delete(Database& db, const nlohmann::json &input) = 0;

protected:
   nlohmann::json CreateInvalidIdResponse() const;
};

#endif // TYPEREQUESTHANDLER_H
