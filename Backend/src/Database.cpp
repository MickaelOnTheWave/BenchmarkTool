#include "Database.h"

#include <fstream>
#include <iterator>

Database::Database(const std::string& path)
{
   dbPath = path;

   if (sqlite3_open(path.c_str(), &db) != SQLITE_OK)
      return;

   if (!IsSchemaInitialized())
      InitSchema();
}

Database::~Database()
{
   if (db)
      sqlite3_close(db);
}

sqlite3* Database::GetHandle() const
{
   return db;
}

std::optional<std::string> Database::Execute(const std::string& sql)
{
   char* err = nullptr;

   if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK)
   {
      std::string msg = err ? err : "Unknown error";
      sqlite3_free(err);
      return msg;
   }

   return std::nullopt;
}

bool Database::QueryInt(const std::string& sql, int& outValue)
{
   sqlite3_stmt* stmt = nullptr;

   if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
      return false;

   if (sqlite3_step(stmt) == SQLITE_ROW)
      outValue = sqlite3_column_int(stmt, 0);
   else
   {
      sqlite3_finalize(stmt);
      return false;
   }

   sqlite3_finalize(stmt);
   return true;
}

int Database::GetLastInsertId() const
{
   return static_cast<int>(sqlite3_last_insert_rowid(db));
}

bool Database::IsSchemaInitialized()
{
   const char* query =
      "SELECT name FROM sqlite_master "
      "WHERE type='table' AND name='Machine';";

   sqlite3_stmt* stmt = nullptr;

   if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK)
      return false;

   bool exists = (sqlite3_step(stmt) == SQLITE_ROW);

   sqlite3_finalize(stmt);

   return exists;
}

std::optional<std::string> Database::InitSchema()
{
   std::ifstream file("schema.sql");

   if (!file.is_open())
      return "Cannot open schema file";

   std::string sql(
      (std::istreambuf_iterator<char>(file)),
      std::istreambuf_iterator<char>()
      );

   return Execute(sql);
}
