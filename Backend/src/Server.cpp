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

   svr.Get("/ui/index.html", rootRequest);
   svr.Get("/api/db-status", dbStatusRequest);
   svr.Get("/api/list-machines", listMachinesRequest);
   svr.Post("/api/create-machine", createMachineRequest);

   std::cout << "Backend server running on http://localhost:8080\n";
   svr.listen("localhost", 8080);
   return 0;
}
