#ifndef ADDFULLMACHINEREQUEST_H
#define ADDFULLMACHINEREQUEST_H

#include <nlohmann/json.hpp>

#include "Database.h"

class AddFullMachineRequest
{
public:
   static nlohmann::json CreateJsonResponse(Database& db, const nlohmann::json &input);

private:
};

#endif // ADDFULLMACHINEREQUEST_H
