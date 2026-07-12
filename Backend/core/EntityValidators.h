#ifndef ENTITYVALIDATORS_H
#define ENTITYVALIDATORS_H

#include <vector>
#include <string>
#include <nlohmann/json.hpp>

using ErrorList = std::vector<std::string>;

ErrorList ValidateMachine(const nlohmann::json& j);
ErrorList ValidateHardwareConfiguration(const nlohmann::json& j);
ErrorList ValidateSoftwareEnvironment(const nlohmann::json& j);
ErrorList ValidateSoftwareConfiguration(const nlohmann::json& j);
ErrorList ValidateTest(const nlohmann::json& j);
ErrorList ValidateTestConfiguration(const nlohmann::json& j);
ErrorList ValidateBenchmarkRun(const nlohmann::json& j);

#endif // ENTITYVALIDATORS_H
