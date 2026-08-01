#ifndef SOFTWARECONFIGREQUESTS_H
#define SOFTWARECONFIGREQUESTS_H

#include <nlohmann/json.hpp>

#include "Database.h"


class SoftwareConfigRequests
{
public:
   static nlohmann::json List(Database& db, const nlohmann::json &input);
   static nlohmann::json Create(Database& db, const nlohmann::json &input);
   static nlohmann::json Delete(Database& db, const nlohmann::json &input);
};

#endif // SOFTWARECONFIGREQUESTS_H
