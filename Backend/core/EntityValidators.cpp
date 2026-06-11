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

ErrorList ValidateBenchmarkRun(const json& j)
{
   ErrorList errors;

   auto requirePositiveInt = [&](const char* key)
   {
      if (!j.contains(key) || !j[key].is_number_integer())
      {
         errors.push_back(std::string(key) + " must be an integer");
         return;
      }

      int v = j[key].get<int>();
      if (v <= 0)
         errors.push_back(std::string(key) + " must be > 0");
   };

   requirePositiveInt("machineId");
   requirePositiveInt("hardwareConfigurationId");
   requirePositiveInt("softwareEnvironmentId");
   requirePositiveInt("softwareConfigurationId");
   requirePositiveInt("testId");
   requirePositiveInt("testConfigurationId");

   if (!j.contains("timestamp") || !j["timestamp"].is_string() || j["timestamp"].get<std::string>().empty())
      errors.push_back("timestamp must be a non-empty string");

   if (!j.contains("result") || !j["result"].is_object())
   {
      errors.push_back("result must be an object");
      return errors;
   }

   const auto& r = j["result"];

   auto requireNumber = [&](const char* key)
   {
      if (!r.contains(key) || !r[key].is_number())
         errors.push_back(std::string(key) + " must be a number");
   };

   requireNumber("avgFps");
   requireNumber("minFps");
   requireNumber("maxFps");
   requireNumber("score");

   if (r.contains("minFps") && r.contains("avgFps") && r.contains("maxFps")
       && r["minFps"].is_number()
       && r["avgFps"].is_number()
       && r["maxFps"].is_number())
   {
      double min = r["minFps"];
      double avg = r["avgFps"];
      double max = r["maxFps"];

      if (!(min <= avg && avg <= max))
         errors.push_back("fps values must satisfy min <= avg <= max");
   }

   return errors;
}
