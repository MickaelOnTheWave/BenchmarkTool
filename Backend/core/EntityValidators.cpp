#include "EntityValidators.h"

using json = nlohmann::json;

ErrorList ValidateMachine(const json& j)
{
   ErrorList errors;

   // name
   if (!j.contains("name") || !j["name"].is_string() || j["name"].get<std::string>().empty())
      errors.push_back("name is required and must be a non-empty string");

   // cpu
   if (!j.contains("cpu") || !j["cpu"].is_string() || j["cpu"].get<std::string>().empty())
      errors.push_back("cpu is required and must be a non-empty string");

   // gpu
   if (!j.contains("gpu") || !j["gpu"].is_string() || j["gpu"].get<std::string>().empty())
      errors.push_back("gpu is required and must be a non-empty string");

   // ramGb
   if (!j.contains("ramGb") || !j["ramGb"].is_number_integer())
      errors.push_back("ramGb is required and must be an integer");
   else
   {
      int ram = j["ramGb"].get<int>();
      if (ram <= 0)
         errors.push_back("ramGb must be > 0");
      else if (ram > 4096)
         errors.push_back("ramGb is unrealistically large");
   }

   // motherboard
   if (!j.contains("motherboard") || !j["motherboard"].is_string() || j["motherboard"].get<std::string>().empty())
      errors.push_back("motherboard is required and must be a non-empty string");

   return errors;
}
