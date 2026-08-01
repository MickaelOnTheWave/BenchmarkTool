#ifndef TESTREQUESTS_H
#define TESTREQUESTS_H

#include <nlohmann/json.hpp>

#include "Database.h"


class TestRequests
{
public:
   static nlohmann::json List(Database& db, const nlohmann::json &input);
   static nlohmann::json Create(Database& db, const nlohmann::json &input);
   static nlohmann::json Delete(Database& db, const nlohmann::json &input);
};

#endif // TESTREQUESTS_H
