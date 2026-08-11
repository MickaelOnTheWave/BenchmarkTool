#include "Server.h"

#include <filesystem>
#include <iostream>

#include "DatabaseHelpers.h"
#include "EntityValidators.h"
#include "FileFormatDetector.h"
#include "Normalizer.h"
#include "requests/AddFullMachineRequest.h"
#include "requests/HardwareConfigRequestHandler.h"
#include "requests/ImportPlanRequest.h"
#include "requests/MachineRequestHandler.h"
#include "requests/SoftwareConfigRequestHandler.h"
#include "requests/SoftwareEnvironmentRequestHandler.h"
#include "requests/SystemInfoRequest.h"
#include "requests/TestConfigRequestHandler.h"
#include "requests/TestRequestHandler.h"
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
{
   const std::string databaseFile = "data/benchmark.db";
   std::filesystem::create_directories("data");
   const bool ok = db.Open(databaseFile);
   if (ok)
   {
      std::cout << "Opened " + databaseFile + " successfully." << std::endl;
      RegisterRoutes();
      server.set_mount_point("/ui", "./ui");
   }
   else
      std::cout << "Error opening " + databaseFile + ". Database not opened." << std::endl;
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

   AddApiGetHandler("/api/get-local-system-info", [](Database& db, const nlohmann::json& input)
   {
      return SystemInfoRequest::CreateJsonResponse();
   });

   AddCrudTypeHandlers("machine", new MachineRequestHandler());
   AddCrudTypeHandlers("hardware-config", new HardwareConfigRequestHandler());
   AddCrudTypeHandlers("software-environment", new SoftwareEnvironmentRequestHandler());
   AddCrudTypeHandlers("software-config", new SoftwareConfigRequestHandler());
   AddCrudTypeHandlers("test", new TestRequestHandler());
   AddCrudTypeHandlers("test-config", new TestConfigRequestHandler());

   AddApiGetHandler("/api/list-origins", [this](Database& db, const nlohmann::json& input)
   {
      return ListOriginsRequest(db, input);
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
/*
   server.Post("/api/import/execute", [this](const httplib::Request& req, httplib::Response& res)
   {
      ExecuteImportPlan(req, res);
   });
*/
   AddApiPostHandler("/api/import/execute", [](Database& db, const nlohmann::json& input)
   {
      return ImportPlanRequest::Execute(db, input);
   });

   AddApiPostHandler("/api/add-full-machine", [](Database& db, const nlohmann::json& input)
   {
      return AddFullMachineRequest::CreateJsonResponse(db, input);
   });

   server.Post("/api/testing/reset", [this](const httplib::Request& req, httplib::Response& res)
   {
      ResetDatabaseRequest(req, res);
   });
}

void Server::AddCrudTypeHandlers(const std::string &typeName, TypeRequestHandler *requestHandler)
{
   auto sharedHandler = std::shared_ptr<TypeRequestHandler>(requestHandler);
   requestHandlers.push_back(sharedHandler);

   AddApiGetHandler("/api/list-" + typeName + "s", [sharedHandler](Database& db, const nlohmann::json& input)
   {
      return sharedHandler->List(db, input);
   });
   AddApiPostHandler("/api/create-" + typeName, [sharedHandler](Database& db, const nlohmann::json& input)
   {
      return sharedHandler->Create(db, input);
   });
   AddApiDeleteHandler("/api/delete-" + typeName, [sharedHandler](Database& db, const nlohmann::json& input)
   {
     return sharedHandler->Delete(db, input);
   });
}

void Server::AddApiGetHandler(const std::string &endpoint, ApiHandler handler)
{
   server.Get(endpoint, CreateHttpGetHandler(handler));
}

void Server::AddApiPostHandler(const std::string &endpoint, ApiHandler handler)
{
   server.Post(endpoint, CreateHttpPostHandler(handler));
}

void Server::AddApiDeleteHandler(const std::string &endpoint, ApiHandler handler)
{
   server.Delete(endpoint, CreateHttpPostHandler(handler));
}

Server::HttpHandler Server::CreateHttpGetHandler(ApiHandler handler)
{
   auto httpHandler = [this, handler](const httplib::Request& req, httplib::Response& res)
   {
      json input;
      const json j = handler(db, input);
      res.set_content(j.dump(3), "application/json");
   };
   return httpHandler;
}

Server::HttpHandler Server::CreateHttpPostHandler(ApiHandler handler)
{
   auto httpHandler = [this, handler](const httplib::Request& req, httplib::Response& res)
   {
      json input;
      try
      {
         input = json::parse(req.body);
      }
      catch (const std::exception&)
      {
         SetHttpResponse400(res, "Invalid JSON input");
         return;
      }

      const json j = handler(db, input);
      res.set_content(j.dump(3), "application/json");
   };
   return httpHandler;
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

json Server::ListOriginsRequest(Database& db, const json& input)
{
   EntityListDescriptor descriptor = EntityHelpers::ListOrigins();
   return DatabaseHelpers::ListEntities(descriptor, db);
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

std::optional<std::string> Server::DeleteEntityById(const std::string& table, int id, int& affectedRows)
{
   db.Execute("PRAGMA foreign_keys = ON;");
   return DeleteByField(db.GetHandle(), table, "Id", id, affectedRows);
}

std::string Server::BuildSqlInsertQuery(const EntityCreateDescriptor& entity)
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

void Server::SetHttpResponse(httplib::Response &res, const int httpStatusCode, const std::string &message)
{
   res.status = httpStatusCode;

   json response;
   response["status"] = "error";
   response["message"] = message;

   res.set_content(response.dump(), "application/json");
}

void Server::SetHttpResponse400(httplib::Response &res, const std::string &message)
{
   SetHttpResponse(res, 400, message);
}
