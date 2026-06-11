#include "Server.h"

#include <filesystem>
#include <iostream>

using json = nlohmann::json;

Server::Server()
   : db("data/benchmark.db")
{
   std::filesystem::create_directories("data");
   RegisterRoutes();
}

void Server::Run()
{
   std::cout << "Backend server running on http://localhost:8080\n";
   server.listen("localhost", 8080);
}

void Server::RegisterRoutes()
{
   server.Get("/ui/index.html", [this](const httplib::Request& req, httplib::Response& res)
   {
      RootRequest(req, res);
   });

   server.Get("/api/db-status", [this](const httplib::Request& req, httplib::Response& res)
   {
      DbStatusRequest(req, res);
   });

   server.Get("/api/list-machines", [this](const httplib::Request& req, httplib::Response& res)
   {
      ListMachinesRequest(req, res);
   });
   server.Post("/api/create-machine", [this](const httplib::Request& req, httplib::Response& res)
   {
      CreateMachineRequest(req, res);
   });

   server.Get("/api/list-hardware-configs", [this](const httplib::Request& req, httplib::Response& res)
   {
      ListHardwareConfigsRequest(req, res);
   });
   server.Post("/api/create-hardware-config", [this](const httplib::Request& req, httplib::Response& res)
   {
      CreateHardwareConfigRequest(req, res);
   });

   server.Get("/api/list-software-environments", [this](const httplib::Request& req, httplib::Response& res)
   {
      ListSoftwareEnvironmentsRequest(req, res);
   });
   server.Post("/api/create-software-environment", [this](const httplib::Request& req, httplib::Response& res)
   {
      CreateSoftwareEnvironmentRequest(req, res);
   });

   server.Get("/api/list-software-configs", [this](const httplib::Request& req, httplib::Response& res)
   {
      ListSoftwareConfigsRequest(req, res);
   });
   server.Post("/api/create-software-config", [this](const httplib::Request& req, httplib::Response& res)
   {
      CreateSoftwareConfigRequest(req, res);
   });

   server.Get("/api/list-tests", [this](const httplib::Request& req, httplib::Response& res)
   {
      ListTestsRequest(req, res);
   });
   server.Post("/api/create-test", [this](const httplib::Request& req, httplib::Response& res)
   {
      CreateTestRequest(req, res);
   });

   server.Get("/api/list-test-configs", [this](const httplib::Request& req, httplib::Response& res)
   {
      ListTestConfigsRequest(req, res);
   });
   server.Post("/api/create-test-config", [this](const httplib::Request& req, httplib::Response& res)
   {
      CreateTestConfigRequest(req, res);
   });
}

void Server::RootRequest(const httplib::Request&, httplib::Response& res)
{
   res.set_content("My C++ HTTP Server!", "text/plain");
}

void Server::DbStatusRequest(const httplib::Request&, httplib::Response& res)
{
   json j;

   int runCount = 0;
   int machineCount = 0;

   db.QueryInt("SELECT COUNT(*) FROM BenchmarkRun;", runCount);
   db.QueryInt("SELECT COUNT(*) FROM Machine;", machineCount);

   j["status"] = "ok";
   j["runs"] = runCount;
   j["machines"] = machineCount;

   res.set_content(j.dump(3), "application/json");
}

void Server::ListMachinesRequest(const httplib::Request&, httplib::Response& res)
{
   EntityDescriptor entity;
   entity.rootField = "machines";
   entity.table = "Machine";
   entity.fields = "Id, Name, Cpu, Gpu, RamGb, Motherboard";
   entity.selectMapper = [](sqlite3_stmt* sqlStatement, json& jsonObj)
   {
      jsonObj["cpu"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
      jsonObj["gpu"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 3));
      jsonObj["ramGb"] = sqlite3_column_int(sqlStatement, 4);
      jsonObj["motherboard"] = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 5));
   };
   ListEntitiesHttp(db, entity, res);
}

void Server::CreateMachineRequest(const httplib::Request& req, httplib::Response& res)
{
   EntityDescriptor machineEntity;
   machineEntity.table = "Machine";
   machineEntity.insertFields = {
      "Name", "Cpu", "Gpu", "RamGb", "Motherboard"
   };

   machineEntity.insertBinder = [](sqlite3_stmt* stmt, const json& j)
   {
      sqlite3_bind_text(stmt, 1, j.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, j.value("cpu", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, j.value("gpu", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 4, j.value("ramGb", 0));
      sqlite3_bind_text(stmt, 5, j.value("motherboard", "").c_str(), -1, SQLITE_TRANSIENT);
   };

   InsertEntityHttp(db, machineEntity, req, res);
}

void Server::ListHardwareConfigsRequest(const httplib::Request&, httplib::Response& res)
{
   EntityDescriptor entity;
   entity.rootField = "configs";
   entity.table = "HardwareConfiguration";
   entity.fields = "Id, Name, MachineId, CpuFreqGhz, GpuFreqMhz, RamFreqMhz, Settings";
   entity.selectMapper = [](sqlite3_stmt* sqlStatement, json& jsonObj)
   {
      jsonObj["machineId"] = sqlite3_column_int(sqlStatement, 2);
      jsonObj["cpuFreqGhz"] = sqlite3_column_double(sqlStatement, 3);
      jsonObj["gpuFreqMhz"] = sqlite3_column_double(sqlStatement, 4);
      jsonObj["ramFreqMhz"] = sqlite3_column_double(sqlStatement, 5);

      const char* settingsText =
         reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 6));

      if (settingsText)
         jsonObj["settings"] = json::parse(settingsText, nullptr, false);
      else
         jsonObj["settings"] = json::object();
   };
   ListEntitiesHttp(db, entity, res);
}

void Server::CreateHardwareConfigRequest(const httplib::Request& req, httplib::Response& res)
{
   EntityDescriptor machineEntity;
   machineEntity.table = "HardwareConfiguration";
   machineEntity.insertFields = {
      "Name", "MachineId", "CpuFreqGhz", "GpuFreqMhz", "RamFreqMhz", "Settings"
   };

   machineEntity.insertBinder = [](sqlite3_stmt* stmt, const json& j)
   {
      sqlite3_bind_text(stmt, 1, j.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 2, j.value("machineId", -1));
      sqlite3_bind_double(stmt, 3, j.value("cpuFreqGhz", 0.0));
      sqlite3_bind_double(stmt, 4, j.value("gpuFreqMhz", 0.0));
      sqlite3_bind_double(stmt, 5, j.value("ramFreqMhz", 0.0));
      sqlite3_bind_text(stmt, 6, j.value("settings", "").c_str(), -1, SQLITE_TRANSIENT);
   };

   InsertEntityHttp(db, machineEntity, req, res);
}

void Server::ListSoftwareEnvironmentsRequest(const httplib::Request&, httplib::Response& res)
{
   EntityDescriptor entity;

   entity.rootField = "softwareEnvironments";
   entity.table = "SoftwareEnvironment";
   entity.fields = "Id, Name, Os, OsVersion, DriverFamily";

   entity.selectMapper = [](sqlite3_stmt* stmt, json& obj)
   {
      obj["os"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
      obj["osVersion"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
      obj["driverFamily"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
   };

   ListEntitiesHttp(db, entity, res);
}

void Server::CreateSoftwareEnvironmentRequest(const httplib::Request& req, httplib::Response& res)
{
   EntityDescriptor entity;
   entity.table = "SoftwareEnvironment";
   entity.insertFields = {
      "Name",
      "Os",
      "OsVersion",
      "DriverFamily"
   };

   entity.insertBinder = [](sqlite3_stmt* stmt, const json& j)
   {
      sqlite3_bind_text(stmt, 1, j.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, j.value("os", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, j.value("osVersion", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 4, j.value("driverFamily", "").c_str(), -1, SQLITE_TRANSIENT);
   };

   InsertEntityHttp(db, entity, req, res);
}

void Server::ListSoftwareConfigsRequest(const httplib::Request&, httplib::Response& res)
{
   EntityDescriptor entity;

   entity.rootField = "softwareConfigurations";
   entity.table = "SoftwareConfiguration";
   entity.fields =
      "Id, Name, SoftwareEnvironmentId, DriverVersion, Mode, Settings";

   entity.selectMapper = [](sqlite3_stmt* stmt, json& obj)
   {
      obj["softwareEnvironmentId"] = sqlite3_column_int(stmt, 2);
      obj["driverVersion"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
      obj["mode"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
      obj["settings"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
   };

   ListEntitiesHttp(db, entity, res);
}

void Server::CreateSoftwareConfigRequest(const httplib::Request& req, httplib::Response& res)
{
   EntityDescriptor entity;
   entity.table = "SoftwareConfiguration";
   entity.insertFields = {
      "Name",
      "SoftwareEnvironmentId",
      "DriverVersion",
      "Mode",
      "Settings"
   };

   entity.insertBinder = [](sqlite3_stmt* stmt, const json& j)
   {
      sqlite3_bind_text(stmt, 1, j.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 2, j.value("softwareEnvironmentId", 0));
      sqlite3_bind_text(stmt, 3, j.value("driverVersion", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 4, j.value("mode", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 5, j.value("settings", "{}").c_str(), -1, SQLITE_TRANSIENT);
   };

   InsertEntityHttp(db, entity, req, res);
}

void Server::ListTestsRequest(const httplib::Request&, httplib::Response& res)
{
   EntityDescriptor entity;

   entity.rootField = "tests";
   entity.table = "Test";
   entity.fields = "Id, Name, Description, IconPath";

   entity.selectMapper = [](sqlite3_stmt* stmt, json& obj)
   {
      obj["description"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
      obj["iconPath"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
   };

   ListEntitiesHttp(db, entity, res);
}

void Server::CreateTestRequest(const httplib::Request& req, httplib::Response& res)
{
   EntityDescriptor entity;
   entity.table = "Test";
   entity.insertFields = {
      "Name",
      "Description",
      "IconPath"
   };

   entity.insertBinder = [](sqlite3_stmt* stmt, const json& j)
   {
      sqlite3_bind_text(stmt, 1, j.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, j.value("description", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, j.value("iconPath", "").c_str(), -1, SQLITE_TRANSIENT);
   };

   InsertEntityHttp(db, entity, req, res);
}

void Server::ListTestConfigsRequest(const httplib::Request&, httplib::Response& res)
{
   EntityDescriptor entity;
   entity.rootField = "testConfigurations";
   entity.table = "TestConfiguration";
   entity.fields = "Id, Name, TestId, Settings";

   entity.selectMapper = [](sqlite3_stmt* stmt, json& obj)
   {
      obj["testId"] = sqlite3_column_int(stmt, 2);
      obj["settings"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
   };

   ListEntitiesHttp(db, entity, res);
}

void Server::CreateTestConfigRequest(const httplib::Request& req, httplib::Response& res)
{
   EntityDescriptor entity;
   entity.table = "TestConfiguration";
   entity.insertFields = {
      "Name",
      "TestId",
      "Settings"
   };

   entity.insertBinder = [](sqlite3_stmt* stmt, const json& j)
   {
      sqlite3_bind_text(stmt, 1, j.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 2, j.value("testId", 0));
      sqlite3_bind_text(stmt, 3, j.value("settings", "{}").c_str(), -1, SQLITE_TRANSIENT);
   };

   InsertEntityHttp(db, entity, req, res);
}

void Server::ListEntitiesHttp(Database& db, Server::EntityDescriptor& entity, httplib::Response& res)
{
   json returnedJson = ListEntities(db, entity);
   if (returnedJson.contains("error"))
      res.status = 500;
   res.set_content(returnedJson.dump(3), "application/json");
}

json Server::ListEntities(Database& db, Server::EntityDescriptor& entity)
{
   json returnedJson;
   returnedJson[entity.rootField] = json::array();

   sqlite3_stmt* sqlStatement = nullptr;
   const std::string selectCommand = "SELECT " + entity.fields + " FROM " + entity.table + ";";

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
      entity.selectMapper(sqlStatement, jsonEntry);

      returnedJson[entity.rootField].push_back(jsonEntry);
   }

   sqlite3_finalize(sqlStatement);
   return returnedJson;
}

void Server::InsertEntityHttp(Database& db, const Server::EntityDescriptor& entity, const httplib::Request& req, httplib::Response& res)
{
   json response;
   json request = json::parse(req.body, nullptr, false);
   if (request.is_discarded())
   {
      res.status = 400;
      response["status"] = "error";
      response["message"] = "Invalid JSON";
      res.set_content(response.dump(), "application/json");
      return;
   }

   auto insertResult = InsertEntity(db, entity, request);
   if (insertResult.has_value())
   {
      res.status = 500;
      response["status"] = "error";
      response["message"] = insertResult.value();
      res.set_content(response.dump(), "application/json");
      return;
   }

   response["status"] = "ok";
   res.set_content(response.dump(), "application/json");
}

std::optional<std::string> Server::InsertEntity(Database& db, const Server::EntityDescriptor& entity, const json& input)
{
   const std::string insertQuery = BuildSqlInsertQuery(entity);

   sqlite3_stmt* stmt = nullptr;

   if (sqlite3_prepare_v2(db.GetHandle(), insertQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
      return sqlite3_errmsg(db.GetHandle());

   entity.insertBinder(stmt, input);

   if (sqlite3_step(stmt) != SQLITE_DONE)
   {
      std::string err = sqlite3_errmsg(db.GetHandle());
      sqlite3_finalize(stmt);
      return err;
   }

   sqlite3_finalize(stmt);
   return std::nullopt;
}

std::string Server::BuildSqlInsertQuery(const Server::EntityDescriptor& entity)
{
   std::string sql = "INSERT INTO " + entity.table + " (";

   for (size_t i = 0; i < entity.insertFields.size(); i++)
   {
      sql += entity.insertFields[i];
      if (i + 1 < entity.insertFields.size())
         sql += ", ";
   }

   sql += ") VALUES (";

   for (size_t i = 0; i < entity.insertFields.size(); i++)
   {
      sql += "?";
      if (i + 1 < entity.insertFields.size())
         sql += ", ";
   }

   sql += ");";
   return sql;
}
