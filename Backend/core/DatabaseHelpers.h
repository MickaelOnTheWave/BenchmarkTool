#ifndef DATABASEHELPERS_H
#define DATABASEHELPERS_H

#include <nlohmann/json.hpp>
#include "Database.h"
#include "EntityHelpers.h"

class DatabaseHelpers
{
public:
   static nlohmann::json ListEntities(const EntityListDescriptor& descriptor, Database& db);
   static nlohmann::json InsertEntity(const EntityCreateDescriptor& descriptor, const nlohmann::json& data, Database& db);
   static nlohmann::json DeleteEntity(const int id, const std::string& table, Database& db);

private:
   static bool TryFormatNotNullConstraintError(const std::string& sqliteMessage, std::string& formattedMessage);
   static std::optional<std::string> InsertInDatabase(const EntityCreateDescriptor& descriptor, Database& db);
   static std::string BuildSqlInsertQuery(const EntityCreateDescriptor& descriptor);

   static bool IsForeignKeyConstraintError(const std::string& message);
};

#endif // DATABASEHELPERS_H
