#ifndef NORMALIZER_H
#define NORMALIZER_H

#include <nlohmann/json.hpp>

class Database;

class Normalizer
{
public:
   explicit Normalizer(Database& _db);

   nlohmann::json Normalize(const nlohmann::json& importedData);

private:
   nlohmann::json NormalizeMachine(const nlohmann::json& importedData);
   nlohmann::json NormalizeHardwareConfig(const int machineId, const nlohmann::json& importedData);
   nlohmann::json NormalizeSoftwareEnvironment(const nlohmann::json& importedData);
   nlohmann::json NormalizeSoftwareConfig(const int softwareEnvironmentId, const nlohmann::json& importedData);
   nlohmann::json NormalizeTest(const nlohmann::json& importedData);
   nlohmann::json NormalizeTestConfig(const int testId, const nlohmann::json& importedData);
   nlohmann::json NormalizeBenchmarkRun(const nlohmann::json& executionPlan, const nlohmann::json& importedData);

   int FindMatchingMachine(const nlohmann::json& machineData);
   int FindMatchingHardwareConfig(const int machineId, const nlohmann::json& machineData);
   int FindMatchingSoftwareEnvironment(const nlohmann::json& envData);
   int FindMatchingSoftwareConfig(const int softwareEnvironmentId, const nlohmann::json& envData);
   int FindMatchingTest(const nlohmann::json& testData);
   int FindMatchingTestConfig(const int testId, const std::string& settings);

   void SetCreateHardwareConfig(const nlohmann::json& machineData, nlohmann::json& result);
   void SetCreateSoftwareEnvironment(const nlohmann::json& envData, nlohmann::json& result);
   void SetCreateSoftwareConfig(const nlohmann::json& envData, nlohmann::json& result);
   void SetCreateTest(const nlohmann::json& testData, nlohmann::json& result);
   void SetCreateTestConfig(const nlohmann::json& testData, nlohmann::json& result);

   static int ResolvePlanId(const nlohmann::json& subPlan);

   // Builds the JSON settings blob describing a test configuration from the
   // imported test data. Used both to create the plan and to dedup existing rows.
   static std::string BuildTestConfigSettings(const nlohmann::json& testData);

   Database& db;
};

#endif // NORMALIZER_H
