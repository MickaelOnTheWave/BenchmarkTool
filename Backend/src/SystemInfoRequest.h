#ifndef SYSTEMINFOREQUEST_H
#define SYSTEMINFOREQUEST_H

#include "ISystemInfo.h"

#include <nlohmann/json.hpp>

class SystemInfoRequest
{
public:
   static nlohmann::json CreateJsonResponse();

private:
   static void SetFieldValues(nlohmann::json& j, const std::string& propertyFamily,
                              const ProcessingUnitInfo& info);
   static void SetFieldValues(nlohmann::json& j, const std::string& propertyFamily,
                              const MemoryInfo& info);
};

#endif // SYSTEMINFOREQUEST_H
