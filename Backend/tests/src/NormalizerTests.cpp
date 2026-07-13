#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include "Database.h"
#include "Normalizer.h"

using json = nlohmann::json;

namespace
{
   // Minimal schema mirroring schema/schema.sql, created directly so the tests
   // do not depend on the schema.sql file being present in the run directory.
   const char* kSchema =
      "PRAGMA foreign_keys = ON;"
      "CREATE TABLE IF NOT EXISTS Machine (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL UNIQUE,"
      " Cpu TEXT, Gpu TEXT, RamGb INTEGER, Motherboard TEXT);"
      "CREATE TABLE IF NOT EXISTS HardwareConfiguration (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL,"
      " MachineId INTEGER NOT NULL, CpuFreqGhz REAL, GpuFreqMhz REAL, RamFreqMhz REAL, Settings TEXT);"
      "CREATE TABLE IF NOT EXISTS SoftwareEnvironment (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL,"
      " Os TEXT NOT NULL, OsVersion TEXT, DriverFamily TEXT NOT NULL);"
      "CREATE TABLE IF NOT EXISTS SoftwareConfiguration (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL,"
      " SoftwareEnvironmentId INTEGER NOT NULL, DriverVersion TEXT NOT NULL, Mode TEXT, Settings TEXT);"
      "CREATE TABLE IF NOT EXISTS Test (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL, Description TEXT, IconPath TEXT);"
      "CREATE TABLE IF NOT EXISTS TestConfiguration (Id INTEGER PRIMARY KEY, Name TEXT NOT NULL,"
      " TestId INTEGER NOT NULL, Settings TEXT);";

   // Seeds the schema above into an existing in-memory DB. Database owns a raw
   // sqlite handle and is non-movable, so callers construct it on the stack and
   // pass it here rather than receiving one by value.
   void SeedSchema(Database& db)
   {
      db.Execute(kSchema);
   }

   // Representative 3DMark-importer-shaped payload.
   json SampleImport()
   {
      json j;
      j["machine"]["cpu"]["name"] = "Intel i7";
      j["machine"]["cpu"]["frequency"] = "3600";
      j["machine"]["gpu"]["name"] = "NVIDIA RTX 4090";
      j["machine"]["gpu"]["frequency"] = "2200";
      j["machine"]["motherboard"]["model"] = "ASUS Z790";
      j["machine"]["ram"] = 32;

      j["softwareenvironment"]["os"] = "Windows 11";
      j["softwareenvironment"]["driverversion"] = "551.23";
      j["softwareenvironment"]["bios"]["version"] = "1.2.3";
      j["softwareenvironment"]["bios"]["date"] = "2024-01-01";

      j["test"]["name"] = "Time Spy";
      j["test"]["usedGpu"] = "NVIDIA RTX 4090";
      j["test"]["textureFilterMode"] = "Anisotropic";
      j["test"]["resolution"] = "2560x1440";

      j["benchmarkRun"]["time"] = "2024-05-01T10:00:00";
      j["benchmarkRun"]["result"]["score"] = 25000;
      j["benchmarkRun"]["origin"]["runExternalId"] = "ext-123";

      return j;
   }
}

TEST_CASE("Normalize - empty DB creates every entity", "[normalizer]")
{
   Database db(":memory:");
   SeedSchema(db);
   Normalizer normalizer(db);

   const json plan = normalizer.Normalize(SampleImport());

   REQUIRE(plan["machine"]["action"] == "create");
   REQUIRE(plan["machine"]["data"]["cpu"] == "Intel i7");
   REQUIRE(plan["machine"]["data"]["gpu"] == "NVIDIA RTX 4090");
   REQUIRE(plan["machine"]["data"]["motherboard"] == "ASUS Z790");
   REQUIRE(plan["machine"]["data"]["ramGb"] == 32);

   REQUIRE(plan["hardwareconfig"]["action"] == "create");
   REQUIRE(plan["hardwareconfig"]["data"]["cpuFreqGhz"] == "3600");
   REQUIRE(plan["hardwareconfig"]["data"]["gpuFreqMhz"] == "2200");

   REQUIRE(plan["softwareenvironment"]["action"] == "create");
   REQUIRE(plan["softwareenvironment"]["data"]["os"] == "Windows 11");
   REQUIRE(plan["softwareenvironment"]["data"]["driverFamily"] == "");

   REQUIRE(plan["softwareconfig"]["action"] == "create");
   REQUIRE(plan["softwareconfig"]["data"]["driverVersion"] == "551.23");

   REQUIRE(plan["test"]["action"] == "create");
   REQUIRE(plan["test"]["data"]["name"] == "Time Spy");

   REQUIRE(plan["testconfig"]["action"] == "create");

   REQUIRE(plan["benchmarkrun"]["action"] == "create");
   REQUIRE(plan["benchmarkrun"]["data"]["timestamp"] == "2024-05-01T10:00:00");
   REQUIRE(plan["benchmarkrun"]["data"]["result"]["score"] == 25000);
   REQUIRE(plan["benchmarkrun"]["data"]["origin"]["externalId"] == "ext-123");
}

TEST_CASE("Normalize - benchmark run references unresolved parents as 0", "[normalizer]")
{
   Database db(":memory:");
   SeedSchema(db);
   Normalizer normalizer(db);

   const json plan = normalizer.Normalize(SampleImport());

   // Nothing exists yet, so every parent is "create" and its id is 0.
   REQUIRE(plan["benchmarkrun"]["data"]["machineId"] == 0);
   REQUIRE(plan["benchmarkrun"]["data"]["softwareEnvironmentId"] == 0);
   REQUIRE(plan["benchmarkrun"]["data"]["testId"] == 0);
}

TEST_CASE("Normalize - reuses existing machine (regression: reuse-id bug)", "[normalizer]")
{
   Database db(":memory:");
   SeedSchema(db);
   db.Execute(
      "INSERT INTO Machine (Name, Cpu, Gpu, RamGb, Motherboard) "
      "VALUES ('m1', 'Intel i7', 'NVIDIA RTX 4090', 32, 'ASUS Z790');");
   const int machineId = db.GetLastInsertId();

   Normalizer normalizer(db);
   const json plan = normalizer.Normalize(SampleImport());

   REQUIRE(plan["machine"]["action"] == "reuse");
   REQUIRE(plan["machine"]["id"] == machineId);

   // Because the machine is reused, the hardware config must be planned against
   // that machine id (this path is what the old ["action"]["id"] bug broke).
   REQUIRE(plan["hardwareconfig"]["action"] == "create");
   REQUIRE(plan["benchmarkrun"]["data"]["machineId"] == machineId);
}

TEST_CASE("Normalize - reuses existing hardware config for reused machine", "[normalizer]")
{
   Database db(":memory:");
   SeedSchema(db);
   db.Execute(
      "INSERT INTO Machine (Name, Cpu, Gpu, RamGb, Motherboard) "
      "VALUES ('m1', 'Intel i7', 'NVIDIA RTX 4090', 32, 'ASUS Z790');");
   const int machineId = db.GetLastInsertId();

   db.Execute(
      "INSERT INTO HardwareConfiguration (Name, MachineId, CpuFreqGhz, GpuFreqMhz) "
      "VALUES ('hw1', " + std::to_string(machineId) + ", 3600, 2200);");
   const int hwId = db.GetLastInsertId();

   Normalizer normalizer(db);
   const json plan = normalizer.Normalize(SampleImport());

   REQUIRE(plan["hardwareconfig"]["action"] == "reuse");
   REQUIRE(plan["hardwareconfig"]["id"] == hwId);
   REQUIRE(plan["benchmarkrun"]["data"]["hardwareConfigurationId"] == hwId);
}

TEST_CASE("Normalize - reuses test and chains test config", "[normalizer]")
{
   Database db(":memory:");
   SeedSchema(db);
   db.Execute("INSERT INTO Test (Name) VALUES ('Time Spy');");
   const int testId = db.GetLastInsertId();

   Normalizer normalizer(db);

   json plan = normalizer.Normalize(SampleImport());

   REQUIRE(plan["test"]["action"] == "reuse");
   REQUIRE(plan["test"]["id"] == testId);
   REQUIRE(plan["testconfig"]["action"] == "create");
   REQUIRE(plan["benchmarkrun"]["data"]["testId"] == testId);

   // Seed the matching test configuration and re-run: it should now be reused.
   const std::string settings =
      json{{"usedGpu", "NVIDIA RTX 4090"},
           {"textureFilterMode", "Anisotropic"},
           {"resolution", "2560x1440"}}
         .dump();

   sqlite3_stmt* stmt = nullptr;
   sqlite3_prepare_v2(db.GetHandle(),
                      "INSERT INTO TestConfiguration (Name, TestId, Settings) VALUES ('tc1', ?, ?);",
                      -1, &stmt, nullptr);
   sqlite3_bind_int(stmt, 1, testId);
   sqlite3_bind_text(stmt, 2, settings.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_step(stmt);
   sqlite3_finalize(stmt);
   const int tcId = db.GetLastInsertId();

   plan = normalizer.Normalize(SampleImport());
   REQUIRE(plan["testconfig"]["action"] == "reuse");
   REQUIRE(plan["testconfig"]["id"] == tcId);
}

TEST_CASE("Normalize - reuses software environment and chains config", "[normalizer]")
{
   Database db(":memory:");
   SeedSchema(db);
   db.Execute(
      "INSERT INTO SoftwareEnvironment (Name, Os, OsVersion, DriverFamily) "
      "VALUES ('env1', 'Windows 11', '', '');");
   const int envId = db.GetLastInsertId();

   Normalizer normalizer(db);
   const json plan = normalizer.Normalize(SampleImport());

   REQUIRE(plan["softwareenvironment"]["action"] == "reuse");
   REQUIRE(plan["softwareenvironment"]["id"] == envId);
   REQUIRE(plan["softwareconfig"]["action"] == "create");
   REQUIRE(plan["benchmarkrun"]["data"]["softwareEnvironmentId"] == envId);
}
