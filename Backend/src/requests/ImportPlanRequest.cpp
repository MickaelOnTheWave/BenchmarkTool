#include "ImportPlanRequest.h"

#include <sqlite3.h>
#include "HardwareConfigRequestHandler.h"
#include "MachineRequestHandler.h"
#include "SoftwareConfigRequestHandler.h"
#include "SoftwareEnvironmentRequestHandler.h"
#include "TestConfigRequestHandler.h"
#include "TestRequestHandler.h"

using json = nlohmann::json;

namespace
{
   json CreateErrorResponse(Database& db, const std::string& message)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      json error;
      error["status"] = "error";
      error["error"]["message"] = message;
      return error;
   }
}

json ImportPlanRequest::Execute(Database &db, const json &input)
{
   // --- Begin transaction -------------------------------------------------
   char* err = nullptr;
   if (sqlite3_exec(db.GetHandle(), "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK)
   {
      json response;
      response["status"] = "error";
      response["error"]["message"] = err ? err : "Failed to start transaction";
      sqlite3_free(err);
      return response;
   }

   json response;

   // --- 1. Machine --------------------------------------------------------
   MachineRequestHandler machineHandler;
   const int machineId = ResolveEntity(db, response, input, "machine", machineHandler);
   if (response.value("status", "") == "error" || machineId <= 0)
      return CreateErrorResponse(db, "Machine not resolved");

   // --- 2. Hardware Configuration -----------------------------------------
   HardwareConfigRequestHandler hwConfHandler;
   const int hwConfId = ResolveEntity(db, response, input, "hardwareconfig", hwConfHandler,
                                      "MachineId", machineId);
   if (response.value("status", "") == "error" || hwConfId <= 0)
      return CreateErrorResponse(db, "Hardware configuration not resolved");

   // --- 3. Software Environment ------------------------------------------
   SoftwareEnvironmentRequestHandler envHandler;
   const int envId = ResolveEntity(db, response, input, "softwareenvironment", envHandler);
   if (response.value("status", "") == "error" || envId <= 0)
      return CreateErrorResponse(db, "Software environment not resolved");

   // --- 4. Software Configuration -----------------------------------------
   SoftwareConfigRequestHandler swConfHandler;
   const int swConfId = ResolveEntity(db, response, input, "softwareconfig", swConfHandler,
                                      "SoftwareEnvironmentId", envId);
   if (response.value("status", "") == "error" || swConfId <= 0)
      return CreateErrorResponse(db, "Software configuration not resolved");

   // --- 5. Test ----------------------------------------------------------
   TestRequestHandler testHandler;
   const int testId = ResolveEntity(db, response, input, "test", testHandler);
   if (response.value("status", "") == "error" || testId <= 0)
      return CreateErrorResponse(db, "Test not resolved");

   // --- 6. Test Configuration ---------------------------------------------
   TestConfigRequestHandler testCfgHandler;
   const int testCfgId = ResolveEntity(db, response, input, "testconfig", testCfgHandler,
                                      "TestId", testId);
   if (response.value("status", "") == "error" || testCfgId <= 0)
      return CreateErrorResponse(db, "Test configuration not resolved");

   // --- 7. Benchmark Run --------------------------------------------------
   const json runData = input.value("benchmarkrun", json::object());
   const std::string timestamp = runData.value("timestamp", "");

   sqlite3_stmt* stmt = nullptr;
   const std::string insertRunQuery =
      "INSERT INTO BenchmarkRun (MachineId, HardwareConfigurationId, SoftwareEnvironmentId, "
      "SoftwareConfigurationId, TestId, TestConfigurationId, Timestamp) "
      "VALUES (?, ?, ?, ?, ?, ?, ?);";

   if (sqlite3_prepare_v2(db.GetHandle(), insertRunQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
   {
      return CreateErrorResponse(db, std::string("BenchmarkRun: ") + sqlite3_errmsg(db.GetHandle()));
   }

   sqlite3_bind_int(stmt, 1, machineId);
   sqlite3_bind_int(stmt, 2, hwConfId);
   sqlite3_bind_int(stmt, 3, envId);
   sqlite3_bind_int(stmt, 4, swConfId);
   sqlite3_bind_int(stmt, 5, testId);
   sqlite3_bind_int(stmt, 6, testCfgId);
   sqlite3_bind_text(stmt, 7, timestamp.c_str(), -1, SQLITE_TRANSIENT);

   if (sqlite3_step(stmt) != SQLITE_DONE)
   {
      std::string errMsg = sqlite3_errmsg(db.GetHandle());
      sqlite3_finalize(stmt);
      return CreateErrorResponse(db, "BenchmarkRun: " + errMsg);
   }

   sqlite3_finalize(stmt);
   const int runId = db.GetLastInsertId();

   // --- 8. Result ---------------------------------------------------------
   const json resultData = runData.value("result", json::object());

   const std::string insertResultQuery =
      "INSERT INTO Result (RunId, AvgFps, MinFps, MaxFps, Score) VALUES (?, ?, ?, ?, ?);";

   if (sqlite3_prepare_v2(db.GetHandle(), insertResultQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
   {
      return CreateErrorResponse(db, std::string("Result: ") + sqlite3_errmsg(db.GetHandle()));
   }

   sqlite3_bind_int(stmt, 1, runId);
   sqlite3_bind_double(stmt, 2, resultData.value("avgFps", 0.0));
   sqlite3_bind_double(stmt, 3, resultData.value("minFps", 0.0));
   sqlite3_bind_double(stmt, 4, resultData.value("maxFps", 0.0));
   sqlite3_bind_double(stmt, 5, resultData.value("score", 0.0));

   if (sqlite3_step(stmt) != SQLITE_DONE)
   {
      std::string errMsg = sqlite3_errmsg(db.GetHandle());
      sqlite3_finalize(stmt);
      return CreateErrorResponse(db, "Result: " + errMsg);
   }

   sqlite3_finalize(stmt);

   // --- 9. Origin ---------------------------------------------------------
   const json originData = runData.value("origin", json::object());

   const std::string insertOriginQuery =
      "INSERT INTO Origin (RunId, OriginType, ExternalId, SourceFile) VALUES (?, ?, ?, ?);";

   if (sqlite3_prepare_v2(db.GetHandle(), insertOriginQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
   {
      return CreateErrorResponse(db, std::string("Origin: ") + sqlite3_errmsg(db.GetHandle()));
   }

   sqlite3_bind_int(stmt, 1, runId);
   sqlite3_bind_text(stmt, 2, "imported", -1, SQLITE_STATIC);
   sqlite3_bind_text(stmt, 3, originData.value("externalId", "").c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 4, originData.value("sourceFile", "").c_str(), -1, SQLITE_TRANSIENT);

   if (sqlite3_step(stmt) != SQLITE_DONE)
   {
      std::string errMsg = sqlite3_errmsg(db.GetHandle());
      sqlite3_finalize(stmt);
      return CreateErrorResponse(db, "Origin: " + errMsg);
   }

   sqlite3_finalize(stmt);

   // --- Commit ------------------------------------------------------------
   if (sqlite3_exec(db.GetHandle(), "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK)
   {
      sqlite3_exec(db.GetHandle(), "ROLLBACK;", nullptr, nullptr, nullptr);
      std::string msg = err ? err : "Failed to commit";
      sqlite3_free(err);
      response["status"] = "error";
      response["error"]["message"] = msg;
      return response;
   }

   response["status"] = "ok";
   response["runId"] = runId;
   return response;
}

int ImportPlanRequest::ResolveEntity(Database& db, json& response, const json& input,
                                    const std::string& planKey, TypeRequestHandler& handler,
                                    const std::string& parentFkField, int parentId)
{
   const json subPlan = input.value(planKey, json::object());
   const std::string action = subPlan.value("action", "");

   // --- Reuse existing entity ---
   if (action == "reuse")
   {
      const int id = subPlan.value("id", 0);
      if (id <= 0)
      {
         response["status"] = "error";
         response["error"]["message"] = planKey + ": reuse requires a valid id";
         return 0;
      }
      return id;
   }

   // --- Invalid action ---
   if (action != "create")
   {
      response["status"] = "error";
      response["error"]["message"] = planKey + ": action must be 'reuse' or 'create'";
      return 0;
   }

   // --- Create new entity ---
   json data = subPlan.value("data", json::object());
   if (!parentFkField.empty() && parentId > 0)
   {
      data[parentFkField] = parentId;
   }

   json result = handler.Create(db, data);
   if (result.value("status", "") != "ok")
   {
      response = result;
      return 0;
   }

   return result.value("id", 0);
}

