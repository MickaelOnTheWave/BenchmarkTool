#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>

#include "Database.h"

using json = nlohmann::json;

int main()
{
   std::cout << "Working directory : " << std::filesystem::current_path() << std::endl;

   httplib::Server svr;

   std::filesystem::create_directories("data");
   Database db("data/benchmark.db");

   auto rootRequest = [](const httplib::Request& req, httplib::Response& res)
   {
      res.set_content("My C++ HTTP Server!", "text/plain");
   };
   auto dbStatusRequest = [&](const httplib::Request&, httplib::Response& res)
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
   };

   auto listMachinesRequest = [&](const httplib::Request&, httplib::Response& res)
   {
     json j;
     j["machines"] = json::array();

     sqlite3_stmt* stmt = nullptr;

     const char* sql =
        "SELECT Id, Name, Cpu, Gpu, RamGb, Motherboard FROM Machine;";

     if (sqlite3_prepare_v2(db.GetHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK)
     {
        json jsonError;
        jsonError["error"] = sqlite3_errmsg(db.GetHandle());

        res.status = 500;
        res.set_content(jsonError.dump(3), "application/json");
        return;
     }

     while (sqlite3_step(stmt) == SQLITE_ROW)
     {
        json m;

        m["id"] = sqlite3_column_int(stmt, 0);
        m["name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        m["cpu"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        m["gpu"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        m["ramGb"] = sqlite3_column_int(stmt, 4);
        m["motherboard"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        j["machines"].push_back(m);
     }

     sqlite3_finalize(stmt);

     res.set_content(j.dump(3), "application/json");
   };

   auto createMachineRequest = [&](const httplib::Request& req, httplib::Response& res)
   {
      json j = json::parse(req.body, nullptr, false);

      if (j.is_discarded())
      {
         res.status = 400;
         res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
         return;
      }

      std::string name = j.value("name", "");
      std::string cpu = j.value("cpu", "");
      std::string gpu = j.value("gpu", "");
      int ramGb = j.value("ramGb", 0);
      std::string motherboard = j.value("motherboard", "");

      if (name.empty())
      {
         res.status = 400;
         res.set_content("{\"error\":\"Missing name\"}", "application/json");
         return;
      }

      // --------------------------
      // Check duplicate name
      // --------------------------
      int existingId = -1;
      if (db.QueryInt("SELECT Id FROM Machine WHERE Name = '" + name + "';", existingId))
      {
         json err;
         err["error"] = "Machine name already exists";
         res.status = 409;
         res.set_content(err.dump(), "application/json");
         return;
      }

      // --------------------------
      // Insert machine
      // --------------------------
      std::string sql =
         "INSERT INTO Machine (Name, Cpu, Gpu, RamGb, Motherboard) VALUES ('" +
         name + "','" +
         cpu + "','" +
         gpu + "'," +
         std::to_string(ramGb) + ",'" +
         motherboard + "');";

      auto err = db.Execute(sql);

      if (err)
      {
         res.status = 500;
         res.set_content("{\"error\":\"Insert failed\"}", "application/json");
         return;
      }

      int id = db.GetLastInsertId();

      json ok;
      ok["id"] = id;

      res.set_content(ok.dump(), "application/json");
   };

   auto createHardwareConfigRequest = [&](const httplib::Request& req, httplib::Response& res)
   {
      json j = json::parse(req.body, nullptr, false);

      if (j.is_discarded())
      {
         res.status = 400;
         res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
         return;
      }

      int machineId = j.value("machineId", -1);
      std::string name = j.value("name", "");

      if (machineId < 0 || name.empty())
      {
         res.status = 400;
         res.set_content("{\"error\":\"Missing fields\"}", "application/json");
         return;
      }

      double cpuFreq = j.value("cpuFreqGhz", 0.0);
      double gpuFreq = j.value("gpuFreqMhz", 0.0);
      double ramFreq = j.value("ramFreqMhz", 0.0);

      json settings = j.value("settings", json::object());

      std::string sql =
         "INSERT INTO HardwareConfiguration "
         "(MachineId, Name, CpuFreqGhz, GpuFreqMhz, RamFreqMhz, Settings) VALUES ("
         + std::to_string(machineId) + ", '" +
         name + "', " +
         std::to_string(cpuFreq) + ", " +
         std::to_string(gpuFreq) + ", " +
         std::to_string(ramFreq) + ", '" +
         settings.dump() + "');";

      auto errorMsg = db.Execute(sql);
      if (errorMsg.has_value())
      {
         json jsonError;
         jsonError["error"] = sqlite3_errmsg(db.GetHandle());

         res.status = 500;
         res.set_content(jsonError.dump(3), "application/json");
         return;
      }

      json out;
      out["id"] = db.GetLastInsertId();

      res.set_content(out.dump(), "application/json");
   };

   auto listHardwareConfigsRequest = [&](const httplib::Request&, httplib::Response& res)
   {
      json j;
      j["configs"] = json::array();

      const char* sql =
         "SELECT Id, MachineId, Name, CpuFreqGhz, GpuFreqMhz, RamFreqMhz, Settings "
         "FROM HardwareConfiguration;";

      sqlite3_stmt* stmt;

      if (sqlite3_prepare_v2(db.GetHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK)
      {
         res.status = 500;
         res.set_content("{\"error\":\"Query failed\"}", "application/json");
         return;
      }

      while (sqlite3_step(stmt) == SQLITE_ROW)
      {
         json c;

         c["id"] = sqlite3_column_int(stmt, 0);
         c["machineId"] = sqlite3_column_int(stmt, 1);
         c["name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
         c["cpuFreqGhz"] = sqlite3_column_double(stmt, 3);
         c["gpuFreqMhz"] = sqlite3_column_double(stmt, 4);
         c["ramFreqMhz"] = sqlite3_column_double(stmt, 5);

         const char* settingsText =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

         if (settingsText)
            c["settings"] = json::parse(settingsText, nullptr, false);
         else
            c["settings"] = json::object();

         j["configs"].push_back(c);
      }

      sqlite3_finalize(stmt);

      res.set_content(j.dump(3), "application/json");
   };

   svr.Get("/ui/index.html", rootRequest);
   svr.Get("/api/db-status", dbStatusRequest);
   svr.Get("/api/list-machines", listMachinesRequest);
   svr.Get("/api/list-hardware-configs", listHardwareConfigsRequest);

   svr.Post("/api/create-machine", createMachineRequest);
   svr.Post("/api/create-hardware-config", createHardwareConfigRequest);

   std::cout << "Backend server running on http://localhost:8080\n";
   svr.listen("localhost", 8080);
   return 0;
}
