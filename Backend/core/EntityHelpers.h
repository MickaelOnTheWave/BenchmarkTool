#ifndef ENTITYHELPERS_H
#define ENTITYHELPERS_H

#include <sqlite3.h>

#include <nlohmann/json.hpp>

using ErrorList = std::vector<std::string>;
struct EntityCreateDescriptor
{
   std::string table;
   std::vector<std::string> insertFields;
   std::function<void(sqlite3_stmt*)> insertBinder;
   std::function<ErrorList(const nlohmann::json&)> validator;
};

struct EntityListDescriptor
{
   std::string table;
   std::string selectFields;
   std::string rootField;
   std::function<void(sqlite3_stmt*, nlohmann::json&)> selectMapper;
};

class EntityHelpers
{
public:
   static EntityListDescriptor ListMachines();
   static EntityListDescriptor ListHardwareConfigs();

   static EntityCreateDescriptor CreateMachine(const nlohmann::json& data);
   static EntityCreateDescriptor CreateHardwareConfig(const nlohmann::json& data);
   static EntityCreateDescriptor CreateSoftwareEnvironment(const nlohmann::json& data);
   static EntityCreateDescriptor CreateSoftwareConfig(const nlohmann::json& data);
   static EntityCreateDescriptor CreateTest(const nlohmann::json& data);
};

#endif // ENTITYHELPERS_H
