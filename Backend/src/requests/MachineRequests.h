#ifndef MACHINEREQUESTS_H
#define MACHINEREQUESTS_H

#include <nlohmann/json.hpp>

#include "Database.h"


class MachineRequests
{
public:
   static nlohmann::json List(Database& db, const nlohmann::json &input);
   static nlohmann::json Create(Database& db, const nlohmann::json &input);
   static nlohmann::json Delete(Database& db, const nlohmann::json &input);
};

#endif // MACHINEREQUESTS_H
