#ifndef ADDFULLMACHINEREQUEST_H
#define ADDFULLMACHINEREQUEST_H

#include <nlohmann/json.hpp>

#include "Database.h"

class AddFullMachineRequest
{
public:
   static nlohmann::json CreateJsonResponse(Database& db, const nlohmann::json &input);

private:
   static nlohmann::json HandleError(Database& db, const nlohmann::json &input);
};

#endif // ADDFULLMACHINEREQUEST_H
