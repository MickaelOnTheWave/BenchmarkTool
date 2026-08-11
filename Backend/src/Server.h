#ifndef SERVER_H
#define SERVER_H

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <sqlite3.h>
#include <string>

#include "Database.h"
#include "EntityHelpers.h"
#include "requests/TypeRequestHandler.h"

class Server
{
public:
   Server();
   void Run();

private:
   void RegisterRoutes();

   void AddCrudTypeHandlers(const std::string& typeName, TypeRequestHandler* requestHandler);

   using ApiHandler = std::function<nlohmann::json(Database& db, const nlohmann::json& input)>;
   using HttpHandler = std::function<void(const httplib::Request& req, httplib::Response& res)>;

   void AddApiGetHandler(const std::string& endpoint, ApiHandler handler);
   void AddApiPostHandler(const std::string& endpoint, ApiHandler handler);
   void AddApiDeleteHandler(const std::string& endpoint, ApiHandler handler);

   HttpHandler CreateHttpGetHandler(ApiHandler handler);
   HttpHandler CreateHttpPostHandler(ApiHandler handler);

   // Request handlers
   void DbStatusRequest(const httplib::Request& req, httplib::Response& res);

   nlohmann::json ListOriginsRequest(Database &db, const nlohmann::json &input);
   void ListBenchmarkRunsRequest(const httplib::Request& req, httplib::Response& res);
   void CreateBenchmarkRunRequest(const httplib::Request& req, httplib::Response& res);
   void DeleteBenchmarkRunRequest(const httplib::Request& req, httplib::Response& res);

   void ResetDatabaseRequest(const httplib::Request& req, httplib::Response& res);

   void ImportFiles(const httplib::Request& req, httplib::Response& res);


   // Helpers
   std::optional<std::string> DeleteEntityById(const std::string& table, int id, int& affectedRows);

   std::string BuildSqlInsertQuery(const EntityCreateDescriptor& entity);

   void SetHttpResponse(httplib::Response& res, const ErrorList& errors);
   void SetHttpResponse(httplib::Response& res, const int httpStatusCode, const std::string& message);
   void SetHttpResponse400(httplib::Response& res, const std::string& message);

   httplib::Server server;
   Database db;
   std::vector<std::shared_ptr<TypeRequestHandler>> requestHandlers;
};

#endif
