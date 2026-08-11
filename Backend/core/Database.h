#ifndef DATABASE_H
#define DATABASE_H

#include <expected>
#include <optional>
#include <sqlite3.h>
#include <string>

class Database
{
public:
   Database() = default;
   Database(const std::string& path);
   virtual ~Database();

   bool Open(const std::string& dbFile);

   sqlite3* GetHandle() const;
   std::optional<std::string> Execute(const std::string& sql);
   std::expected<int, std::string> Delete(const std::string& table, const int id);

   bool QueryInt(const std::string& sql, int& outValue);

   int GetLastInsertId() const;

private:
   bool IsSchemaInitialized();
   std::optional<std::string> InitSchema();

   sqlite3* db = nullptr;
   std::string dbPath;
};

#endif
