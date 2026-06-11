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
   struct EntityDescriptor
   {
      std::string table;
      std::string fields;
      std::string rootField;

      std::function<void(sqlite3_stmt*, nlohmann::json&)> selectMapper;

      std::vector<std::string> insertFields;
      std::function<void(sqlite3_stmt*, const nlohmann::json&)> insertBinder;
   };

   void RegisterRoutes();

   // Request handlers
   void RootRequest(const httplib::Request& req, httplib::Response& res);
   void DbStatusRequest(const httplib::Request& req, httplib::Response& res);
   void ListMachinesRequest(const httplib::Request& req, httplib::Response& res);
   void CreateMachineRequest(const httplib::Request& req, httplib::Response& res);
   void ListHardwareConfigsRequest(const httplib::Request& req, httplib::Response& res);
   void CreateHardwareConfigRequest(const httplib::Request& req, httplib::Response& res);
   void ListSoftwareEnvironmentsRequest(const httplib::Request& req, httplib::Response& res);
   void CreateSoftwareEnvironmentRequest(const httplib::Request& req, httplib::Response& res);
   void ListSoftwareConfigsRequest(const httplib::Request& req, httplib::Response& res);
   void CreateSoftwareConfigRequest(const httplib::Request& req, httplib::Response& res);
   void ListTestsRequest(const httplib::Request& req, httplib::Response& res);
   void CreateTestRequest(const httplib::Request& req, httplib::Response& res);
   void ListTestConfigsRequest(const httplib::Request& req, httplib::Response& res);
   void CreateTestConfigRequest(const httplib::Request& req, httplib::Response& res);

   // Helpers
   void ListEntitiesHttp(Database& db, Server::EntityDescriptor& entity, httplib::Response& res);
   nlohmann::json ListEntities(Database& db, Server::EntityDescriptor& entity);

   void InsertEntityHttp(Database& db, const Server::EntityDescriptor& entity, const httplib::Request& req, httplib::Response& res);
   std::optional<std::string> InsertEntity(Database& db, const Server::EntityDescriptor& entity, const nlohmann::json& input);

   std::string BuildSqlInsertQuery(const Server::EntityDescriptor& entity);

   httplib::Server server;
   Database db;
};

#endif
