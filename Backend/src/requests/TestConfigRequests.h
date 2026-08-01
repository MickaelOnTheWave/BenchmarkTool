#ifndef TESTCONFIGREQUESTS_H
#define TESTCONFIGREQUESTS_H

#include <nlohmann/json.hpp>

#include "Database.h"


class TestConfigRequests
{
public:
   static nlohmann::json List(Database& db, const nlohmann::json &input);
   static nlohmann::json Create(Database& db, const nlohmann::json &input);
   static nlohmann::json Delete(Database& db, const nlohmann::json &input);
};

#endif // TESTCONFIGREQUESTS_H
