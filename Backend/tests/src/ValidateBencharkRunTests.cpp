#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include "EntityValidators.h"
#include "TestHelpers.h"

using json = nlohmann::json;

TEST_CASE("ValidateBenchmarkRun - valid input", "[validation][run]")
{
   json j = {
      {"machineId", 1},
      {"hardwareConfigurationId", 2},
      {"softwareEnvironmentId", 3},
      {"softwareConfigurationId", 4},
      {"testId", 5},
      {"testConfigurationId", 6},
      {"timestamp", "2026-01-01T00:00:00Z"},
      {"result", {
                    {"score", 1000},
                    {"avgFps", 100},
                    {"minFps", 80},
                    {"maxFps", 120}
                 }}
   };

   auto errors = ValidateBenchmarkRun(j);
   REQUIRE(errors.empty());
}

TEST_CASE("ValidateBenchmarkRun - missing machineId", "[validation][run]")
{
   json j = {
      {"hardwareConfigurationId", 2},
      {"softwareEnvironmentId", 3},
      {"softwareConfigurationId", 4},
      {"testId", 5},
      {"testConfigurationId", 6},
      {"timestamp", "2026-01-01T00:00:00Z"},
      {"result", {
                    {"score", 1000},
                    {"avgFps", 100},
                    {"minFps", 80},
                    {"maxFps", 120}
                 }}
   };

   EXPECT_SINGLE_ERROR(ValidateBenchmarkRun(j), "machineId");
}

TEST_CASE("ValidateBenchmarkRun - missing hardwareConfigurationId", "[validation][run]")
{
   json j = {
      {"machineId", 1},
      {"softwareEnvironmentId", 3},
      {"softwareConfigurationId", 4},
      {"testId", 5},
      {"testConfigurationId", 6},
      {"timestamp", "2026-01-01T00:00:00Z"},
      {"result", {
                    {"score", 1000},
                    {"avgFps", 100},
                    {"minFps", 80},
                    {"maxFps", 120}
                 }}
   };

   EXPECT_SINGLE_ERROR(ValidateBenchmarkRun(j), "hardwareConfigurationId");
}

TEST_CASE("ValidateBenchmarkRun - missing softwareEnvironmentId", "[validation][run]")
{
   json j = {
      {"machineId", 1},
      {"hardwareConfigurationId", 2},
      {"softwareConfigurationId", 4},
      {"testId", 5},
      {"testConfigurationId", 6},
      {"timestamp", "2026-01-01T00:00:00Z"},
      {"result", {
                    {"score", 1000},
                    {"avgFps", 100},
                    {"minFps", 80},
                    {"maxFps", 120}
                 }}
   };

   EXPECT_SINGLE_ERROR(ValidateBenchmarkRun(j), "softwareEnvironmentId");
}

TEST_CASE("ValidateBenchmarkRun - missing softwareConfigurationId", "[validation][run]")
{
   json j = {
      {"machineId", 1},
      {"hardwareConfigurationId", 2},
      {"softwareEnvironmentId", 3},
      {"testId", 5},
      {"testConfigurationId", 6},
      {"timestamp", "2026-01-01T00:00:00Z"},
      {"result", {
                    {"score", 1000},
                    {"avgFps", 100},
                    {"minFps", 80},
                    {"maxFps", 120}
                 }}
   };

   EXPECT_SINGLE_ERROR(ValidateBenchmarkRun(j), "softwareConfigurationId");
}

TEST_CASE("ValidateBenchmarkRun - missing testId", "[validation][run]")
{
   json j = {
      {"machineId", 1},
      {"hardwareConfigurationId", 2},
      {"softwareEnvironmentId", 3},
      {"softwareConfigurationId", 4},
      {"testConfigurationId", 6},
      {"timestamp", "2026-01-01T00:00:00Z"},
      {"result", {
                    {"score", 1000},
                    {"avgFps", 100},
                    {"minFps", 80},
                    {"maxFps", 120}
                 }}
   };

   EXPECT_SINGLE_ERROR(ValidateBenchmarkRun(j), "testId");
}

TEST_CASE("ValidateBenchmarkRun - missing testConfigurationId", "[validation][run]")
{
   json j = {
      {"machineId", 1},
      {"hardwareConfigurationId", 2},
      {"softwareEnvironmentId", 3},
      {"softwareConfigurationId", 4},
      {"testId", 5},
      {"timestamp", "2026-01-01T00:00:00Z"},
      {"result", {
                    {"score", 1000},
                    {"avgFps", 100},
                    {"minFps", 80},
                    {"maxFps", 120}
                 }}
   };

   EXPECT_SINGLE_ERROR(ValidateBenchmarkRun(j), "testConfigurationId");
}

TEST_CASE("ValidateBenchmarkRun - missing result", "[validation][run]")
{
   json j = {
      {"machineId", 1},
      {"hardwareConfigurationId", 2},
      {"softwareEnvironmentId", 3},
      {"softwareConfigurationId", 4},
      {"testId", 5},
      {"testConfigurationId", 6},
      {"timestamp", "2026-01-01T00:00:00Z"}
   };

   EXPECT_SINGLE_ERROR(ValidateBenchmarkRun(j), "result");
}

TEST_CASE("ValidateBenchmarkRun - invalid FPS order", "[validation][run]")
{
   json j = {
      {"machineId", 1},
      {"hardwareConfigurationId", 2},
      {"softwareEnvironmentId", 3},
      {"softwareConfigurationId", 4},
      {"testId", 5},
      {"testConfigurationId", 6},
      {"timestamp", "2026-01-01T00:00:00Z"},
      {"result", {
                    {"score", 1000},
                    {"avgFps", 100},
                    {"minFps", 150},
                    {"maxFps", 120}
                 }}
   };

   EXPECT_SINGLE_ERROR(ValidateBenchmarkRun(j), "fps");
}

TEST_CASE("ValidateBenchmarkRun - invalid ID types", "[validation][run]")
{
   json j = {
      {"machineId", "one"},
      {"hardwareConfigurationId", true},
      {"softwareEnvironmentId", nullptr},
      {"softwareConfigurationId", 4},
      {"testId", 5},
      {"testConfigurationId", 6},
      {"timestamp", "2026-01-01T00:00:00Z"},
      {"result", {
                    {"score", 1000},
                    {"avgFps", 100},
                    {"minFps", 80},
                    {"maxFps", 120}
                 }}
   };

   auto errors = ValidateBenchmarkRun(j);

   REQUIRE_FALSE(errors.size() == 3);
}
