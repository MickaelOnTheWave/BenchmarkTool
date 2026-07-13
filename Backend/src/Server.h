#ifndef SERVER_H
#define SERVER_H

#include <functional>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <sqlite3.h>
#include <string>
#include <vector>

#include "Database.h"

class Server
{
public:
   Server();
   void Run();

private:
   using ErrorList = std::vector<std::string>;
   struct EntityDescriptor
   {
      std::string table;
      std::string fields;
      std::string rootField;

      std::function<void(sqlite3_stmt*, nlohmann::json&)> selectMapper;

      std::vector<std::string> insertFields;
      std::function<void(sqlite3_stmt*, const nlohmann::json&)> insertBinder;
      std::function<ErrorList(const nlohmann::json&)> validator;
   };

   void RegisterRoutes();

   // Request handlers
   void DbStatusRequest(const httplib::Request& req, httplib::Response& res);

   void ListMachinesRequest(const httplib::Request& req, httplib::Response& res);
   void CreateMachineRequest(const httplib::Request& req, httplib::Response& res);
   void DeleteMachineRequest(const httplib::Request& req, httplib::Response& res);
   void ListHardwareConfigsRequest(const httplib::Request& req, httplib::Response& res);
   void CreateHardwareConfigRequest(const httplib::Request& req, httplib::Response& res);
   void DeleteHardwareConfigRequest(const httplib::Request& req, httplib::Response& res);
   void ListSoftwareEnvironmentsRequest(const httplib::Request& req, httplib::Response& res);
   void CreateSoftwareEnvironmentRequest(const httplib::Request& req, httplib::Response& res);
   void DeleteSoftwareEnvironmentRequest(const httplib::Request& req, httplib::Response& res);
   void ListSoftwareConfigsRequest(const httplib::Request& req, httplib::Response& res);
   void CreateSoftwareConfigRequest(const httplib::Request& req, httplib::Response& res);
   void DeleteSoftwareConfigRequest(const httplib::Request& req, httplib::Response& res);
   void ListTestsRequest(const httplib::Request& req, httplib::Response& res);
   void CreateTestRequest(const httplib::Request& req, httplib::Response& res);
   void DeleteTestRequest(const httplib::Request& req, httplib::Response& res);
   void ListTestConfigsRequest(const httplib::Request& req, httplib::Response& res);
   void CreateTestConfigRequest(const httplib::Request& req, httplib::Response& res);
   void DeleteTestConfigRequest(const httplib::Request& req, httplib::Response& res);

   void ListBenchmarkRunsRequest(const httplib::Request& req, httplib::Response& res);
   void CreateBenchmarkRunRequest(const httplib::Request& req, httplib::Response& res);
   void DeleteBenchmarkRunRequest(const httplib::Request& req, httplib::Response& res);

   void ResetDatabaseRequest(const httplib::Request& req, httplib::Response& res);

   void ImportFiles(const httplib::Request& req, httplib::Response& res);
   void ExecuteImportPlan(const httplib::Request& req, httplib::Response& res);

   // Helpers
   void ListEntitiesHttp(Database& db, Server::EntityDescriptor& entity, httplib::Response& res);
   nlohmann::json ListEntities(Database& db, Server::EntityDescriptor& entity);

   void InsertEntityHttp(Database& db, const Server::EntityDescriptor& entity, const httplib::Request& req, httplib::Response& res);
   std::optional<std::string> InsertEntity(Database& db, const Server::EntityDescriptor& entity, const nlohmann::json& input);

   void DeleteEntityHttp(const httplib::Request& req, httplib::Response& res, const std::string& table);
   std::optional<std::string> DeleteEntityById(const std::string& table, int id, int& affectedRows);

   std::string BuildSqlInsertQuery(const Server::EntityDescriptor& entity);

   void SetHttpResponse(httplib::Response& res, const ErrorList& errors);

   httplib::Server server;
   Database db;
};

#endif
