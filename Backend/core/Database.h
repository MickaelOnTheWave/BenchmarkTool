#ifndef DATABASE_H
#define DATABASE_H

#include <optional>
#include <sqlite3.h>
#include <string>

class Database
{
public:
   Database(const std::string& path);
   virtual ~Database();

   sqlite3* GetHandle() const;
   std::optional<std::string> Execute(const std::string& sql);

   bool QueryInt(const std::string& sql, int& outValue);

   int GetLastInsertId() const;

private:
   bool IsSchemaInitialized();
   std::optional<std::string> InitSchema();

   sqlite3* db = nullptr;
   std::string dbPath;
};

#endif
