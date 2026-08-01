#ifndef SOFTWAREENVIRONMENTREQUESTS_H
#define SOFTWAREENVIRONMENTREQUESTS_H

#include <nlohmann/json.hpp>

#include "Database.h"

class SoftwareEnvironmentRequests
{
public:
   static nlohmann::json List(Database& db, const nlohmann::json &input);
   static nlohmann::json Create(Database& db, const nlohmann::json &input);
   static nlohmann::json Delete(Database& db, const nlohmann::json &input);
};

#endif // SOFTWAREENVIRONMENTREQUESTS_H
