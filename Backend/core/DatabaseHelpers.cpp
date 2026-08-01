#include "DatabaseHelpers.h"

using json = nlohmann::json;

json DatabaseHelpers::InsertEntity(const EntityCreateDescriptor& descriptor, const nlohmann::json& data, Database& db)
{
   json status;

   ErrorList validationErrors;
   if (descriptor.validator)
      validationErrors = descriptor.validator(data);

   if (!validationErrors.empty())
   {
      status["status"] = "error";
      status["error"]["message"] = "Validation failed";
      for (const auto& error : validationErrors)
         status["error"]["data"].push_back(error);
      return status;
   }

   auto insertResult = InsertInDatabase(descriptor, db);
   if (insertResult.has_value())
   {
      status["status"] = "error";

      std::string message = insertResult.value();
      std::string formattedMessage;
      if (TryFormatNotNullConstraintError(message, formattedMessage))
      {
         message = formattedMessage;
      }
      status["error"]["message"] = message;
      return status;
   }

   status["status"] = "ok";
   status["id"] = db.GetLastInsertId();
   return status;
}

nlohmann::json DatabaseHelpers::ListEntities(const EntityListDescriptor &descriptor, Database &db)
{
   json returnedJson;
   returnedJson[descriptor.rootField] = json::array();

   sqlite3_stmt* sqlStatement = nullptr;
   const std::string selectCommand = "SELECT " + descriptor.selectFields + " FROM " + descriptor.table + ";";

   if (sqlite3_prepare_v2(db.GetHandle(), selectCommand.c_str(), -1, &sqlStatement, nullptr) != SQLITE_OK)
   {
      json jsonError;
      jsonError["error"] = sqlite3_errmsg(db.GetHandle());
      return jsonError;
   }

   while (sqlite3_step(sqlStatement) == SQLITE_ROW)
   {
      json jsonEntry;

      // Contract:
      // column 0 = id
      // column 1 = name
      assert(sqlite3_column_count(sqlStatement) >= 2);

      jsonEntry["id"] = sqlite3_column_int(sqlStatement, 0);
      jsonEntry["name"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 1));
      descriptor.selectMapper(sqlStatement, jsonEntry);

      returnedJson[descriptor.rootField].push_back(jsonEntry);
   }

   sqlite3_finalize(sqlStatement);
   return returnedJson;
}

nlohmann::json DatabaseHelpers::DeleteEntity(const int id, const std::string &table, Database &db)
{
   json status;

   db.Execute("PRAGMA foreign_keys = ON;");
   const auto deleteResult = db.Delete(table, id);
   if (deleteResult)
   {
      const int affectedRows = *deleteResult;
      if (affectedRows > 0)
         status["status"] = "ok";
      else
      {
         status["status"] = "error";
         status["error"]["message"] = "Entity not found";
      }
   }
   else
   {
      status["status"] = "error";
      if (IsForeignKeyConstraintError(deleteResult.error()))
         status["error"]["message"] = "Entity is still referenced";
      else
      {
         status["error"]["message"] = "Could not delete entry";
         status["error"]["entryId"] = deleteResult.error();
      }
   }
   return status;
}

bool DatabaseHelpers::TryFormatNotNullConstraintError(const std::string &sqliteMessage, std::string &formattedMessage)
{
   const std::string prefix = "NOT NULL constraint failed: ";
   const size_t prefixPos = sqliteMessage.find(prefix);
   if (prefixPos == std::string::npos)
      return false;

   std::string field = sqliteMessage.substr(prefixPos + prefix.size());
   const size_t dotPos = field.rfind('.');
   if (dotPos != std::string::npos)
      field = field.substr(dotPos + 1);

   formattedMessage = "missing data for field " + field;
   return true;
}

std::optional<std::string> DatabaseHelpers::InsertInDatabase(const EntityCreateDescriptor &descriptor, Database &db)
{
   const std::string insertQuery = BuildSqlInsertQuery(descriptor);

   sqlite3_stmt* stmt = nullptr;

   if (sqlite3_prepare_v2(db.GetHandle(), insertQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
      return sqlite3_errmsg(db.GetHandle());

   descriptor.insertBinder(stmt);

   if (sqlite3_step(stmt) != SQLITE_DONE)
   {
      std::string err = sqlite3_errmsg(db.GetHandle());
      sqlite3_finalize(stmt);
      return err;
   }

   sqlite3_finalize(stmt);
   return std::nullopt;
}

std::string DatabaseHelpers::BuildSqlInsertQuery(const EntityCreateDescriptor& descriptor)
{
   std::string sql = "INSERT INTO " + descriptor.table + " (";

   for (size_t i = 0; i < descriptor.insertFields.size(); i++)
   {
      sql += descriptor.insertFields[i];
      if (i + 1 < descriptor.insertFields.size())
         sql += ", ";
   }

   sql += ") VALUES (";

   for (size_t i = 0; i < descriptor.insertFields.size(); i++)
   {
      sql += "?";
      if (i + 1 < descriptor.insertFields.size())
         sql += ", ";
   }

   sql += ");";
   return sql;
}

bool DatabaseHelpers::IsForeignKeyConstraintError(const std::string &message)
{
   return message.find("FOREIGN KEY constraint failed") != std::string::npos;
}
