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

   int FindMatchingMachine(const nlohmann::json& machineData);
   int FindMatchingHardwareConfig(const int machineId, const nlohmann::json& machineData);

   void SetCreateHardwareConfig(const nlohmann::json& machineData, nlohmann::json& result);

   Database& db;
};

#endif // NORMALIZER_H
