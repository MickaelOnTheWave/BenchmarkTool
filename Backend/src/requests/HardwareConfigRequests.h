#ifndef HARDWARECONFIGREQUESTS_H
#define HARDWARECONFIGREQUESTS_H

#include <nlohmann/json.hpp>

#include "Database.h"


class HardwareConfigRequests
{
public:
   static nlohmann::json List(Database& db, const nlohmann::json &input);
   static nlohmann::json Create(Database& db, const nlohmann::json &input);
   static nlohmann::json Delete(Database& db, const nlohmann::json &input);
};

#endif // HARDWARECONFIGREQUESTS_H
