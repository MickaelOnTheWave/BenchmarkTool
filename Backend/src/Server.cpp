#include "Server.h"

#include <filesystem>
#include <iostream>

#include "EntityValidators.h"
#include "FileFormatDetector.h"
#include "Normalizer.h"
#include "ThreeDMarkImporter.h"

using json = nlohmann::json;

namespace
{
   inline ByteBuffer ToByteBuffer(const std::string& data)
   {
      return ByteBuffer(data.begin(), data.end());
   }

   bool TryParsePathId(const httplib::Request& req, int& id)
   {
      if (req.matches.size() < 2)
         return false;

      try
      {
         size_t parsedChars = 0;
         const std::string rawId = req.matches[1].str();
         id = std::stoi(rawId, &parsedChars);
         return parsedChars == rawId.size() && id > 0;
      }
      catch (...)
      {
         return false;
      }
   }

   void SetJsonError(httplib::Response& res, int status, const std::string& message)
   {
      json response;
      response["status"] = "error";
      response["message"] = message;

      res.status = status;
      res.set_content(response.dump(), "application/json");
   }

   void SetJsonOk(httplib::Response& res)
   {
      json response;
      response["status"] = "ok";
      res.set_content(response.dump(), "application/json");
   }

   bool IsForeignKeyConstraintError(const std::string& message)
   {
      return message.find("FOREIGN KEY constraint failed") != std::string::npos;
   }

   bool TryFormatNotNullConstraintError(const std::string& sqliteMessage, std::string& formattedMessage)
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

   std::optional<std::string> DeleteByField(sqlite3* db, const std::string& table, const std::string& field, int id, int& affectedRows)
   {
      const std::string sql = "DELETE FROM " + table + " WHERE " + field + " = ?;";
      sqlite3_stmt* stmt = nullptr;

      if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
         return sqlite3_errmsg(db);

      sqlite3_bind_int(stmt, 1, id);

      if (sqlite3_step(stmt) != SQLITE_DONE)
      {
         std::string err = sqlite3_errmsg(db);
         sqlite3_finalize(stmt);
         return err;
      }

      affectedRows = sqlite3_changes(db);
      sqlite3_finalize(stmt);
      return std::nullopt;
   }
}

Server::Server()
   : db("data/benchmark.db")
{
   std::filesystem::create_directories("data");
   RegisterRoutes();

   server.set_mount_point("/ui", "./ui");
}

void Server::Run()
{
   std::cout << "Backend server running on http://localhost:8080\n";
   server.listen("localhost", 8080);
}

void Server::RegisterRoutes()
{
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
   server.Delete(R"(/api/delete-machine/(\d+))", [this](const httplib::Request& req, httplib::Response& res)
   {
      DeleteMachineRequest(req, res);
   });

   server.Get("/api/list-hardware-configs", [this](const httplib::Request& req, httplib::Response& res)
   {
      ListHardwareConfigsRequest(req, res);
   });
   server.Post("/api/create-hardware-config", [this](const httplib::Request& req, httplib::Response& res)
   {
      CreateHardwareConfigRequest(req, res);
   });
   server.Delete(R"(/api/delete-hardware-config/(\d+))", [this](const httplib::Request& req, httplib::Response& res)
   {
      DeleteHardwareConfigRequest(req, res);
   });

   server.Get("/api/list-software-environments", [this](const httplib::Request& req, httplib::Response& res)
   {
      ListSoftwareEnvironmentsRequest(req, res);
   });
   server.Post("/api/create-software-environment", [this](const httplib::Request& req, httplib::Response& res)
   {
      CreateSoftwareEnvironmentRequest(req, res);
   });
   server.Delete(R"(/api/delete-software-environment/(\d+))", [this](const httplib::Request& req, httplib::Response& res)
   {
      DeleteSoftwareEnvironmentRequest(req, res);
   });

   server.Get("/api/list-software-configs", [this](const httplib::Request& req, httplib::Response& res)
   {
      ListSoftwareConfigsRequest(req, res);
   });
   server.Post("/api/create-software-config", [this](const httplib::Request& req, httplib::Response& res)
   {
      CreateSoftwareConfigRequest(req, res);
   });
   server.Delete(R"(/api/delete-software-config/(\d+))", [this](const httplib::Request& req, httplib::Response& res)
   {
      DeleteSoftwareConfigRequest(req, res);
   });

   server.Get("/api/list-tests", [this](const httplib::Request& req, httplib::Response& res)
   {
      ListTestsRequest(req, res);
   });
   server.Post("/api/create-test", [this](const httplib::Request& req, httplib::Response& res)
   {
      CreateTestRequest(req, res);
   });
   server.Delete(R"(/api/delete-test/(\d+))", [this](const httplib::Request& req, httplib::Response& res)
   {
      DeleteTestRequest(req, res);
   });

   server.Get("/api/list-test-configs", [this](const httplib::Request& req, httplib::Response& res)
   {
      ListTestConfigsRequest(req, res);
   });
   server.Post("/api/create-test-config", [this](const httplib::Request& req, httplib::Response& res)
   {
      CreateTestConfigRequest(req, res);
   });
   server.Delete(R"(/api/delete-test-config/(\d+))", [this](const httplib::Request& req, httplib::Response& res)
   {
      DeleteTestConfigRequest(req, res);
   });

   server.Get("/api/run/list", [this](const httplib::Request& req, httplib::Response& res)
   {
      ListBenchmarkRunsRequest(req, res);
   });
   server.Post("/api/run/create", [this](const httplib::Request& req, httplib::Response& res)
   {
      CreateBenchmarkRunRequest(req, res);
   });
   server.Delete(R"(/api/delete-run/(\d+))", [this](const httplib::Request& req, httplib::Response& res)
   {
      DeleteBenchmarkRunRequest(req, res);
   });

   server.Post("/api/import/files", [this](const httplib::Request& req, httplib::Response& res)
   {
      ImportFiles(req, res);
   });

   server.Post("/api/import/execute", [this](const httplib::Request& req, httplib::Response& res)
   {
      ExecuteImportPlan(req, res);
   });

   server.Post("/api/testing/reset", [this](const httplib::Request& req, httplib::Response& res)
   {
      ResetDatabaseRequest(req, res);
   });
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
   machineEntity.validator = ValidateMachine;

   InsertEntityHttp(db, machineEntity, req, res);
}

void Server::DeleteMachineRequest(const httplib::Request& req, httplib::Response& res)
{
   DeleteEntityHttp(req, res, "Machine");
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

   machineEntity.validator = ValidateHardwareConfiguration;

   InsertEntityHttp(db, machineEntity, req, res);
}

void Server::DeleteHardwareConfigRequest(const httplib::Request& req, httplib::Response& res)
{
   DeleteEntityHttp(req, res, "HardwareConfiguration");
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
   entity.validator = ValidateSoftwareEnvironment;

   InsertEntityHttp(db, entity, req, res);
}

void Server::DeleteSoftwareEnvironmentRequest(const httplib::Request& req, httplib::Response& res)
{
   DeleteEntityHttp(req, res, "SoftwareEnvironment");
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

   entity.validator = ValidateSoftwareConfiguration;

   InsertEntityHttp(db, entity, req, res);
}

void Server::DeleteSoftwareConfigRequest(const httplib::Request& req, httplib::Response& res)
{
   DeleteEntityHttp(req, res, "SoftwareConfiguration");
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

   //entity.validator = ValidateTest;

   InsertEntityHttp(db, entity, req, res);
}

void Server::DeleteTestRequest(const httplib::Request& req, httplib::Response& res)
{
   DeleteEntityHttp(req, res, "Test");
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

   //entity.validator = ValidateTestConfig;

   InsertEntityHttp(db, entity, req, res);
}

void Server::DeleteTestConfigRequest(const httplib::Request& req, httplib::Response& res)
{
   DeleteEntityHttp(req, res, "TestConfiguration");
}

void Server::ListBenchmarkRunsRequest(const httplib::Request& req, httplib::Response& res)
{
   json response;
   response["runs"] = json::array();

   const char* sql = R"(
      SELECT
          br.Id,
          br.Timestamp,

          m.Name,
          hc.Name,
          se.Name,
          sc.Name,
          t.Name,
          tc.Name,

          r.AvgFps,
          r.MinFps,
          r.MaxFps,
          r.Score

      FROM BenchmarkRun br
      JOIN Machine m ON br.MachineId = m.Id
      JOIN HardwareConfiguration hc ON br.HardwareConfigurationId = hc.Id
      JOIN SoftwareEnvironment se ON br.SoftwareEnvironmentId = se.Id
      JOIN SoftwareConfiguration sc ON br.SoftwareConfigurationId = sc.Id
      JOIN Test t ON br.TestId = t.Id
      JOIN TestConfiguration tc ON br.TestConfigurationId = tc.Id
      JOIN Result r ON r.RunId = br.Id
      ORDER BY br.Id DESC;
   )";

   sqlite3_stmt* stmt = nullptr;

   if (sqlite3_prepare_v2(db.GetHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK)
   {
      res.status = 500;
      res.set_content(R"({"status":"error","message":"failed to prepare query"})",
                      "application/json");
      return;
   }

   while (sqlite3_step(stmt) == SQLITE_ROW)
   {
      json run;

      run["id"] = sqlite3_column_int(stmt, 0);
      run["timestamp"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

      run["machine"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
      run["hardwareConfiguration"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
      run["softwareEnvironment"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
      run["softwareConfiguration"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
      run["test"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
      run["testConfiguration"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

      json result;
      result["avgFps"] = sqlite3_column_double(stmt, 8);
      result["minFps"] = sqlite3_column_double(stmt, 9);
      result["maxFps"] = sqlite3_column_double(stmt, 10);
      result["score"] = sqlite3_column_double(stmt, 11);

      run["result"] = result;

      response["runs"].push_back(run);
   }

   sqlite3_finalize(stmt);

   res.set_content(response.dump(3), "application/json");
}

void Server::CreateBenchmarkRunRequest(const httplib::Request& req, httplib::Response& res)
{
   json response;
   json entityJsonData = json::parse(req.body, nullptr, false);
   if (entityJsonData.is_discarded())
   {
      res.status = 400;
      response["status"] = "error";
      response["message"] = "Invalid JSON";
      res.set_content(response.dump(), "application/json");
      return;
   }

   const ErrorList validationErrors = ValidateBenchmarkRun(entityJsonData);
   if (!validationErrors.empty())
   {
      SetHttpResponse(res, validationErrors);
      return;
   }

   char* err = nullptr;
   if (sqlite3_exec(db.GetHandle(), "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK)
   {
      res.status = 500;
      response["status"] = "error";
      response["message"] = err ? err : "Failed to start transaction";
      sqlite3_free(err);
      res.set_content(response.dump(), "application/json");
      return;
   }

   sqlite3_stmt* stmt = nullptr;
   const std::string insertRunQuery =
      "INSERT INTO BenchmarkRun (MachineId, HardwareConfigurationId, SoftwareEnvironmentId, "
      "SoftwareConfigurationId, TestId, TestConfigurationId, Timestamp) "
      "VALUES (?, ?, ?, ?, ?, ?, ?);";

   if (sqlite3_prepare_v2(db.GetHandle(), insertRunQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      res.status = 500;
      response["status"] = "error";
      response["message"] = sqlite3_errmsg(db.GetHandle());
      sqlite3_finalize(stmt);
      res.set_content(response.dump(), "application/json");
      return;
   }

   sqlite3_bind_int(stmt, 1, entityJsonData["machineId"].get<int>());
   sqlite3_bind_int(stmt, 2, entityJsonData["hardwareConfigurationId"].get<int>());
   sqlite3_bind_int(stmt, 3, entityJsonData["softwareEnvironmentId"].get<int>());
   sqlite3_bind_int(stmt, 4, entityJsonData["softwareConfigurationId"].get<int>());
   sqlite3_bind_int(stmt, 5, entityJsonData["testId"].get<int>());
   sqlite3_bind_int(stmt, 6, entityJsonData["testConfigurationId"].get<int>());
   sqlite3_bind_text(stmt, 7, entityJsonData["timestamp"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);

   if (sqlite3_step(stmt) != SQLITE_DONE)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      sqlite3_finalize(stmt);
      res.status = 500;
      response["status"] = "error";
      response["message"] = sqlite3_errmsg(db.GetHandle());
      res.set_content(response.dump(), "application/json");
      return;
   }

   sqlite3_finalize(stmt);

   int runId = db.GetLastInsertId();

   json resultData = entityJsonData["result"];
   const std::string insertResultQuery =
      "INSERT INTO Result (RunId, AvgFps, MinFps, MaxFps, Score) VALUES (?, ?, ?, ?, ?);";

   if (sqlite3_prepare_v2(db.GetHandle(), insertResultQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      res.status = 500;
      response["status"] = "error";
      response["message"] = sqlite3_errmsg(db.GetHandle());
      res.set_content(response.dump(), "application/json");
      return;
   }

   sqlite3_bind_int(stmt, 1, runId);
   sqlite3_bind_double(stmt, 2, resultData.value("avgFps", 0.0));
   sqlite3_bind_double(stmt, 3, resultData.value("minFps", 0.0));
   sqlite3_bind_double(stmt, 4, resultData.value("maxFps", 0.0));
   sqlite3_bind_double(stmt, 5, resultData.value("score", 0.0));

   if (sqlite3_step(stmt) != SQLITE_DONE)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      sqlite3_finalize(stmt);
      res.status = 500;
      response["status"] = "error";
      response["message"] = sqlite3_errmsg(db.GetHandle());
      res.set_content(response.dump(), "application/json");
      return;
   }

   sqlite3_finalize(stmt);

   if (sqlite3_exec(db.GetHandle(), "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      res.status = 500;
      response["status"] = "error";
      response["message"] = err ? err : "Failed to commit transaction";
      sqlite3_free(err);
      res.set_content(response.dump(), "application/json");
      return;
   }

   response["status"] = "ok";
   response["runId"] = runId;
   res.set_content(response.dump(), "application/json");
}

void Server::DeleteBenchmarkRunRequest(const httplib::Request& req, httplib::Response& res)
{
   int id = 0;
   if (!TryParsePathId(req, id))
   {
      SetJsonError(res, 400, "Invalid id");
      return;
   }

   char* err = nullptr;
   if (sqlite3_exec(db.GetHandle(), "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK)
   {
      const std::string message = err ? err : "Failed to start transaction";
      sqlite3_free(err);
      SetJsonError(res, 500, message);
      return;
   }

   int affectedRows = 0;
   auto deleteResult = DeleteByField(db.GetHandle(), "Origin", "RunId", id, affectedRows);
   if (!deleteResult.has_value())
      deleteResult = DeleteByField(db.GetHandle(), "Result", "RunId", id, affectedRows);
   if (!deleteResult.has_value())
      deleteResult = DeleteByField(db.GetHandle(), "BenchmarkRun", "Id", id, affectedRows);

   if (deleteResult.has_value())
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      SetJsonError(res, 500, deleteResult.value());
      return;
   }

   if (affectedRows == 0)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      SetJsonError(res, 404, "Entity not found");
      return;
   }

   if (sqlite3_exec(db.GetHandle(), "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      const std::string message = err ? err : "Failed to commit transaction";
      sqlite3_free(err);
      SetJsonError(res, 500, message);
      return;
   }

   SetJsonOk(res);
}

void Server::ResetDatabaseRequest(const httplib::Request& req, httplib::Response& res)
{
   if (!req.has_param("confirm") || req.get_param_value("confirm") != "yes")
   {
      SetJsonError(res, 400, "Reset requires confirm=yes");
      return;
   }

   const std::vector<std::string> tables = {
      "Origin",
      "Result",
      "BenchmarkRun",
      "TestConfiguration",
      "Test",
      "SoftwareConfiguration",
      "SoftwareEnvironment",
      "HardwareConfiguration",
      "Machine"
   };

   char* err = nullptr;
   if (sqlite3_exec(db.GetHandle(), "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK)
   {
      const std::string message = err ? err : "Failed to start transaction";
      sqlite3_free(err);
      SetJsonError(res, 500, message);
      return;
   }

   for (const auto& table : tables)
   {
      const std::string sql = "DELETE FROM " + table + ";";
      if (sqlite3_exec(db.GetHandle(), sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK)
      {
         sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
         const std::string message = err ? err : "Failed to reset database";
         sqlite3_free(err);
         SetJsonError(res, 500, message);
         return;
      }
   }

   if (sqlite3_exec(db.GetHandle(), "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      const std::string message = err ? err : "Failed to commit transaction";
      sqlite3_free(err);
      SetJsonError(res, 500, message);
      return;
   }

   SetJsonOk(res);
}

void Server::ImportFiles(const httplib::Request& req, httplib::Response& res)
{
   json response;

   if (req.files.empty())
   {
      res.status = 400;
      response["status"] = "error";
      response["message"] = "No files provided";
      res.set_content(response.dump(), "application/json");
      return;
   }

   json filesInfo = json::array();

   for (const auto& [name, file] : req.files)
   {
      json f;
      f["name"] = file.filename;
      f["size"] = file.content.size();

      const ByteBuffer fileData = ToByteBuffer(file.content);
      const ImportFormat fileFormat = FileFormatDetector::Detect(file.filename, fileData);
      if (fileFormat == ImportFormat::_3DMark)
      {
         f["format"] = "3DMark";

         ThreeDMarkImporter importer;
         const json parsedData = importer.Import(fileData);

         Normalizer normalizer(db);
         const json actionData = normalizer.Normalize(parsedData);


         f["parsedData"] = parsedData;
         f["actionData"] = actionData;
      }
      else
      {
         f["format"] = "unknown";
      }

      filesInfo.push_back(f);
   }

   response["status"] = "ok";
   response["files"] = filesInfo;

   std::cout << "Result : " << std::endl;
   std::cout << response.dump(2) << std::endl;

   res.set_content(response.dump(), "application/json");
}

void Server::ExecuteImportPlan(const httplib::Request& req, httplib::Response& res)
{
   json input = json::parse(req.body, nullptr, false);
   if (input.is_discarded())
   {
      SetJsonError(res, 400, "Invalid JSON");
      return;
   }

   // --- Begin transaction -------------------------------------------------
   char* err = nullptr;
   if (sqlite3_exec(db.GetHandle(), "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK)
   {
      std::string msg = err ? err : "Failed to start transaction";
      sqlite3_free(err);
      SetJsonError(res, 500, msg);
      return;
   }

   // Helper: resolve one entity from its sub-plan. Returns the id (existing
   // or newly created), or 0 on failure (with the error response already set).
   auto resolveEntity = [&](const std::string& planKey,
                            const EntityDescriptor& descriptor,
                            const std::string& parentFkField = "",
                            int parentId = 0) -> int
   {
      const json subPlan = input.value(planKey, json::object());
      const std::string action = subPlan.value("action", "");

      if (action == "reuse")
      {
         const int id = subPlan.value("id", 0);
         if (id <= 0)
         {
            sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
            SetJsonError(res, 400, planKey + ": reuse requires a valid id");
         }
         return id;
      }

      if (action != "create")
      {
         sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
         SetJsonError(res, 400, planKey + ": action must be 'reuse' or 'create'");
         return 0;
      }

      // Build the data to insert: start from the plan's data, inject parent FK.
      json data = subPlan.value("data", json::object());
      if (!parentFkField.empty() && parentId > 0)
         data[parentFkField] = parentId;

      auto insertErr = InsertEntity(db, descriptor, data);
      if (insertErr.has_value())
      {
         sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
         SetJsonError(res, 500, planKey + ": " + insertErr.value());
         return 0;
      }

      return db.GetLastInsertId();
   };

   // --- 1. Machine --------------------------------------------------------
   EntityDescriptor machineDesc;
   machineDesc.table = "Machine";
   machineDesc.insertFields = {"Name", "Cpu", "Gpu", "RamGb", "Motherboard"};
   machineDesc.insertBinder = [](sqlite3_stmt* stmt, const json& j)
   {
      sqlite3_bind_text(stmt, 1, j.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, j.value("cpu", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, j.value("gpu", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 4, j.value("ramGb", 0));
      sqlite3_bind_text(stmt, 5, j.value("motherboard", "").c_str(), -1, SQLITE_TRANSIENT);
   };

   const int machineId = resolveEntity("machine", machineDesc);
   if (machineId <= 0) return;

   // --- 2. Hardware Configuration -----------------------------------------
   EntityDescriptor hwDesc;
   hwDesc.table = "HardwareConfiguration";
   hwDesc.insertFields = {"Name", "MachineId", "CpuFreqGhz", "GpuFreqMhz", "RamFreqMhz", "Settings"};
   hwDesc.insertBinder = [](sqlite3_stmt* stmt, const json& j)
   {
      sqlite3_bind_text(stmt, 1, j.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 2, j.value("machineId", 0));
      sqlite3_bind_double(stmt, 3, j.value("cpuFreqGhz", 0.0));
      sqlite3_bind_double(stmt, 4, j.value("gpuFreqMhz", 0.0));
      sqlite3_bind_double(stmt, 5, j.value("ramFreqMhz", 0.0));
      sqlite3_bind_text(stmt, 6, j.value("settings", "").c_str(), -1, SQLITE_TRANSIENT);
   };

   const int hwConfigId = resolveEntity("hardwareconfig", hwDesc, "machineId", machineId);
   if (hwConfigId <= 0) return;

   // --- 3. Software Environment -------------------------------------------
   EntityDescriptor envDesc;
   envDesc.table = "SoftwareEnvironment";
   envDesc.insertFields = {"Name", "Os", "OsVersion", "DriverFamily"};
   envDesc.insertBinder = [](sqlite3_stmt* stmt, const json& j)
   {
      sqlite3_bind_text(stmt, 1, j.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, j.value("os", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, j.value("osVersion", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 4, j.value("driverFamily", "").c_str(), -1, SQLITE_TRANSIENT);
   };

   const int envId = resolveEntity("softwareenvironment", envDesc);
   if (envId <= 0) return;

   // --- 4. Software Configuration -----------------------------------------
   EntityDescriptor swDesc;
   swDesc.table = "SoftwareConfiguration";
   swDesc.insertFields = {"Name", "SoftwareEnvironmentId", "DriverVersion", "Mode", "Settings"};
   swDesc.insertBinder = [](sqlite3_stmt* stmt, const json& j)
   {
      sqlite3_bind_text(stmt, 1, j.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 2, j.value("softwareEnvironmentId", 0));
      sqlite3_bind_text(stmt, 3, j.value("driverVersion", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 4, j.value("mode", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 5, j.value("settings", "{}").c_str(), -1, SQLITE_TRANSIENT);
   };

   const int swConfigId = resolveEntity("softwareconfig", swDesc, "softwareEnvironmentId", envId);
   if (swConfigId <= 0) return;

   // --- 5. Test -----------------------------------------------------------
   EntityDescriptor testDesc;
   testDesc.table = "Test";
   testDesc.insertFields = {"Name", "Description", "IconPath"};
   testDesc.insertBinder = [](sqlite3_stmt* stmt, const json& j)
   {
      sqlite3_bind_text(stmt, 1, j.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, j.value("description", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, j.value("iconPath", "").c_str(), -1, SQLITE_TRANSIENT);
   };

   const int testId = resolveEntity("test", testDesc);
   if (testId <= 0) return;

   // --- 6. Test Configuration ---------------------------------------------
   EntityDescriptor testCfgDesc;
   testCfgDesc.table = "TestConfiguration";
   testCfgDesc.insertFields = {"Name", "TestId", "Settings"};
   testCfgDesc.insertBinder = [](sqlite3_stmt* stmt, const json& j)
   {
      sqlite3_bind_text(stmt, 1, j.value("name", "").c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 2, j.value("testId", 0));
      sqlite3_bind_text(stmt, 3, j.value("settings", "{}").c_str(), -1, SQLITE_TRANSIENT);
   };

   const int testCfgId = resolveEntity("testconfig", testCfgDesc, "testId", testId);
   if (testCfgId <= 0) return;

   // --- 7. Benchmark Run --------------------------------------------------
   const json runData = input.value("benchmarkrun", json::object());

   sqlite3_stmt* stmt = nullptr;
   const std::string insertRunQuery =
      "INSERT INTO BenchmarkRun (MachineId, HardwareConfigurationId, SoftwareEnvironmentId, "
      "SoftwareConfigurationId, TestId, TestConfigurationId, Timestamp) "
      "VALUES (?, ?, ?, ?, ?, ?, ?);";

   if (sqlite3_prepare_v2(db.GetHandle(), insertRunQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      SetJsonError(res, 500, std::string("BenchmarkRun: ") + sqlite3_errmsg(db.GetHandle()));
      return;
   }

   sqlite3_bind_int(stmt, 1, machineId);
   sqlite3_bind_int(stmt, 2, hwConfigId);
   sqlite3_bind_int(stmt, 3, envId);
   sqlite3_bind_int(stmt, 4, swConfigId);
   sqlite3_bind_int(stmt, 5, testId);
   sqlite3_bind_int(stmt, 6, testCfgId);
   sqlite3_bind_text(stmt, 7, runData.value("timestamp", "").c_str(), -1, SQLITE_TRANSIENT);

   if (sqlite3_step(stmt) != SQLITE_DONE)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      std::string errMsg = sqlite3_errmsg(db.GetHandle());
      sqlite3_finalize(stmt);
      SetJsonError(res, 500, "BenchmarkRun: " + errMsg);
      return;
   }

   sqlite3_finalize(stmt);
   const int runId = db.GetLastInsertId();

   // --- 8. Result ---------------------------------------------------------
   const json resultData = runData.value("result", json::object());

   const std::string insertResultQuery =
      "INSERT INTO Result (RunId, AvgFps, MinFps, MaxFps, Score) VALUES (?, ?, ?, ?, ?);";

   if (sqlite3_prepare_v2(db.GetHandle(), insertResultQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      SetJsonError(res, 500, std::string("Result: ") + sqlite3_errmsg(db.GetHandle()));
      return;
   }

   sqlite3_bind_int(stmt, 1, runId);
   sqlite3_bind_double(stmt, 2, resultData.value("avgFps", 0.0));
   sqlite3_bind_double(stmt, 3, resultData.value("minFps", 0.0));
   sqlite3_bind_double(stmt, 4, resultData.value("maxFps", 0.0));
   sqlite3_bind_double(stmt, 5, resultData.value("score", 0.0));

   if (sqlite3_step(stmt) != SQLITE_DONE)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      std::string errMsg = sqlite3_errmsg(db.GetHandle());
      sqlite3_finalize(stmt);
      SetJsonError(res, 500, "Result: " + errMsg);
      return;
   }

   sqlite3_finalize(stmt);

   // --- 9. Origin ---------------------------------------------------------
   const json originData = runData.value("origin", json::object());

   const std::string insertOriginQuery =
      "INSERT INTO Origin (RunId, OriginType, ExternalId, SourceFile) VALUES (?, ?, ?, ?);";

   if (sqlite3_prepare_v2(db.GetHandle(), insertOriginQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      SetJsonError(res, 500, std::string("Origin: ") + sqlite3_errmsg(db.GetHandle()));
      return;
   }

   sqlite3_bind_int(stmt, 1, runId);
   sqlite3_bind_text(stmt, 2, "imported", -1, SQLITE_STATIC);
   sqlite3_bind_text(stmt, 3, originData.value("externalId", "").c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 4, originData.value("sourceFile", "").c_str(), -1, SQLITE_TRANSIENT);

   if (sqlite3_step(stmt) != SQLITE_DONE)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      std::string errMsg = sqlite3_errmsg(db.GetHandle());
      sqlite3_finalize(stmt);
      SetJsonError(res, 500, "Origin: " + errMsg);
      return;
   }

   sqlite3_finalize(stmt);

   // --- Commit ------------------------------------------------------------
   if (sqlite3_exec(db.GetHandle(), "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      std::string msg = err ? err : "Failed to commit";
      sqlite3_free(err);
      SetJsonError(res, 500, msg);
      return;
   }

   json response;
   response["status"] = "ok";
   response["runId"] = runId;
   res.set_content(response.dump(), "application/json");
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
   json entityJsonData = json::parse(req.body, nullptr, false);
   if (entityJsonData.is_discarded())
   {
      res.status = 400;
      response["status"] = "error";
      response["message"] = "Invalid JSON";
      res.set_content(response.dump(), "application/json");
      return;
   }

   ErrorList validationErrors;
   if (entity.validator)
      validationErrors = entity.validator(entityJsonData);

   if (!validationErrors.empty())
   {
      SetHttpResponse(res, validationErrors);
      return;
   }

   auto insertResult = InsertEntity(db, entity, entityJsonData);
   if (insertResult.has_value())
   {
      std::string message = insertResult.value();
      std::string formattedMessage;
      if (TryFormatNotNullConstraintError(message, formattedMessage))
      {
         res.status = 400;
         message = formattedMessage;
      }
      else
      {
         res.status = 500;
      }

      response["status"] = "error";
      response["message"] = message;
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

void Server::DeleteEntityHttp(const httplib::Request& req, httplib::Response& res, const std::string& table)
{
   int id = 0;
   if (!TryParsePathId(req, id))
   {
      SetJsonError(res, 400, "Invalid id");
      return;
   }

   int affectedRows = 0;
   const auto deleteResult = DeleteEntityById(table, id, affectedRows);
   if (deleteResult.has_value())
   {
      if (IsForeignKeyConstraintError(deleteResult.value()))
      {
         SetJsonError(res, 409, "Entity is still referenced");
         return;
      }

      SetJsonError(res, 500, deleteResult.value());
      return;
   }

   if (affectedRows == 0)
   {
      SetJsonError(res, 404, "Entity not found");
      return;
   }

   SetJsonOk(res);
}

std::optional<std::string> Server::DeleteEntityById(const std::string& table, int id, int& affectedRows)
{
   db.Execute("PRAGMA foreign_keys = ON;");
   return DeleteByField(db.GetHandle(), table, "Id", id, affectedRows);
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

void Server::SetHttpResponse(httplib::Response& res, const ErrorList& errors)
{
   res.status = 400;

   json response;
   response["status"] = "error";
   response["message"] = "Validation failed";

   response["errors"] = json::array();
   for (const auto& err : errors)
      response["errors"].push_back(err);

   res.set_content(response.dump(), "application/json");
}
