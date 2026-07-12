#include "EntityValidators.h"

using json = nlohmann::json;

namespace
{
   void RequireString(const json& j, ErrorList& errors, const char* key)
   {
      if (!j.contains(key) || !j[key].is_string() || j[key].get<std::string>().empty())
         errors.push_back(std::string(key) + " is required and must be a non-empty string");
   }

   void CheckOptionalString(const json& j, ErrorList& errors, const char* key)
   {
      if (j.contains(key) && !j[key].is_string())
         errors.push_back(std::string(key) + " must be a string");
   };

   void RequirePositiveInt(const json& j, ErrorList& errors, const char* key)
   {
      if (!j.contains(key) || !j[key].is_number_integer())
      {
         errors.push_back(std::string(key) + " is required and must be an integer");
         return;
      }
      else
      {
         const int v = j[key].get<int>();
         if (v <= 0)
            errors.push_back(std::string(key) + " must be > 0");
      }
   }

   void RequireNumber(const json& j, ErrorList& errors, const char* key)
   {
      if (!j.contains(key) || !j[key].is_number())
         errors.push_back(std::string(key) + " is required and must be a number");
   };

   void CheckOptionalNumber(const json& j, ErrorList& errors, const char* key)
   {
      if (j.contains(key) && !j[key].is_number())
         errors.push_back(std::string(key) + " must be a number");
   }

   void CheckOptionalJson(const json& j, ErrorList& errors, const char* key)
   {
      if (j.contains(key))
      {
         if (!j[key].is_string())
         {
            errors.push_back(std::string(key) + " must be a string containing JSON");
         }
         else
         {
            const auto& s = j[key].get<std::string>();

            nlohmann::json parsed = nlohmann::json::parse(s, nullptr, false);
            if (parsed.is_discarded())
               errors.push_back(std::string(key) + " must be valid JSON");
         }
      }
   }

   bool IsValidNumber(const json& j, const char* key)
   {
      return j.contains(key) && j[key].is_number();
   }

}

ErrorList ValidateMachine(const json& j)
{
   ErrorList errors;
   RequireString(j, errors, "name");
   RequireString(j, errors, "cpu");
   RequireString(j, errors, "gpu");
   RequirePositiveInt(j, errors, "ramGb");
   RequireString(j, errors, "motherboard");
   return errors;
}

ErrorList ValidateHardwareConfiguration(const nlohmann::json& j)
{
   ErrorList errors;
   RequireString(j, errors, "name");
   RequirePositiveInt(j, errors,  "machineId");
   CheckOptionalNumber(j, errors, "cpuFreqGhz");
   CheckOptionalNumber(j, errors, "gpuFreqMhz");
   CheckOptionalNumber(j, errors, "ramFreqMhz");
   CheckOptionalJson(j, errors, "settings");
   return errors;
}

ErrorList ValidateBenchmarkRun(const json& j)
{
   ErrorList errors;

   RequirePositiveInt(j, errors, "machineId");
   RequirePositiveInt(j, errors, "hardwareConfigurationId");
   RequirePositiveInt(j, errors, "softwareEnvironmentId");
   RequirePositiveInt(j, errors, "softwareConfigurationId");
   RequirePositiveInt(j, errors, "testId");
   RequirePositiveInt(j, errors, "testConfigurationId");

   if (!j.contains("timestamp") || !j["timestamp"].is_string() || j["timestamp"].get<std::string>().empty())
      errors.push_back("timestamp must be a non-empty string");

   if (!j.contains("result") || !j["result"].is_object())
   {
      errors.push_back("result must be an object");
      return errors;
   }

   const auto& r = j["result"];
   RequireNumber(r, errors, "avgFps");
   RequireNumber(r, errors, "minFps");
   RequireNumber(r, errors, "maxFps");
   RequireNumber(r, errors, "score");

   if (IsValidNumber(r, "minFps") && IsValidNumber(r, "avgFps") && IsValidNumber(r, "maxFps"))
   {
      const double min = r["minFps"];
      const double avg = r["avgFps"];
      const double max = r["maxFps"];

      if (!(min <= avg && avg <= max))
         errors.push_back("fps values must satisfy min <= avg <= max");
   }

   return errors;
}

ErrorList ValidateSoftwareEnvironment(const nlohmann::json& j)
{
   ErrorList errors;
   RequireString(j, errors, "name");
   RequireString(j, errors, "os");
   RequireString(j, errors, "driverFamily");
   CheckOptionalString(j, errors, "osVersion");
   return errors;
}

ErrorList ValidateSoftwareConfiguration(const nlohmann::json& j)
{
   ErrorList errors;
   RequireString(j, errors, "name");
   RequirePositiveInt(j, errors, "softwareEnvironmentId");
   RequireString(j, errors, "driverVersion");
   CheckOptionalString(j, errors, "mode");
   CheckOptionalJson(j, errors, "settings");
   return errors;
}

ErrorList ValidateTest(const nlohmann::json& j)
{
   ErrorList errors;
   RequireString(j, errors, "name");
   CheckOptionalString(j, errors, "description");
   CheckOptionalString(j, errors, "iconPath");
   return errors;
}

ErrorList ValidateTestConfiguration(const nlohmann::json& j)
{
   ErrorList errors;
   RequireString(j, errors, "name");
   RequirePositiveInt(j, errors, "testId");
   CheckOptionalJson(j, errors, "settings");
   return errors;
}
