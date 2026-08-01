#ifndef SERVER_H
#define SERVER_H

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <sqlite3.h>
#include <string>

#include "Database.h"
#include "EntityHelpers.h"

class Server
{
public:
   Server();
   void Run();

private:
   void RegisterRoutes();

   using ApiHandler = std::function<nlohmann::json(Database& db, const nlohmann::json& input)>;
   using HttpHandler = std::function<void(const httplib::Request& req, httplib::Response& res)>;

   void AddApiGetHandler(const std::string& endpoint, ApiHandler handler);
   void AddApiPostHandler(const std::string& endpoint, ApiHandler handler);
   void AddApiDeleteHandler(const std::string& endpoint, ApiHandler handler);

   HttpHandler CreateHttpHandler(ApiHandler handler);

   // Request handlers
   void DbStatusRequest(const httplib::Request& req, httplib::Response& res);

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

   void ListOriginsRequest(const httplib::Request& req, httplib::Response& res);
   void ListBenchmarkRunsRequest(const httplib::Request& req, httplib::Response& res);
   void CreateBenchmarkRunRequest(const httplib::Request& req, httplib::Response& res);
   void DeleteBenchmarkRunRequest(const httplib::Request& req, httplib::Response& res);

   void ResetDatabaseRequest(const httplib::Request& req, httplib::Response& res);

   void ImportFiles(const httplib::Request& req, httplib::Response& res);
   void ExecuteImportPlan(const httplib::Request& req, httplib::Response& res);
   void AddFullMachineRequest(const httplib::Request& req, httplib::Response& res);

   // Helpers
   void InsertEntityHttp(Database& db, const EntityCreateDescriptor& entity, const httplib::Request& req, httplib::Response& res);
   std::optional<std::string> InsertEntity(Database& db, const EntityCreateDescriptor& entity, const nlohmann::json& input);

   void DeleteEntityHttp(const httplib::Request& req, httplib::Response& res, const std::string& table);
   std::optional<std::string> DeleteEntityById(const std::string& table, int id, int& affectedRows);

   std::string BuildSqlInsertQuery(const EntityCreateDescriptor& entity);

   void SetHttpResponse(httplib::Response& res, const ErrorList& errors);
   void SetHttpResponse(httplib::Response& res, const int httpStatusCode, const std::string& message);
   void SetHttpResponse400(httplib::Response& res, const std::string& message);

   httplib::Server server;
   Database db;
};

#endif
