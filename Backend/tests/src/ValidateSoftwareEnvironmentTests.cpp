#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include "EntityValidators.h"
#include "TestHelpers.h"

using json = nlohmann::json;

TEST_CASE("ValidateSoftwareEnvironment - valid", "[validation][env]")
{
   json j = {
      {"name", "Gaming Env"},
      {"os", "Windows"},
      {"osVersion", "11"},
      {"driverFamily", "NVIDIA"}
   };

   REQUIRE(ValidateSoftwareEnvironment(j).empty());
}

TEST_CASE("ValidateSoftwareEnvironment - missing name", "[validation][env]")
{
   json j = {
      {"os", "Windows"},
      {"osVersion", "11"},
      {"driverFamily", "NVIDIA"}
   };

   EXPECT_SINGLE_ERROR(ValidateSoftwareEnvironment(j), "name");
}

TEST_CASE("ValidateSoftwareEnvironment - missing os", "[validation][env]")
{
   json j = {
      {"name", "Env"},
      {"osVersion", "11"},
      {"driverFamily", "NVIDIA"}
   };

   EXPECT_SINGLE_ERROR(ValidateSoftwareEnvironment(j), "os");
}

TEST_CASE("ValidateSoftwareEnvironment - missing driverFamily", "[validation][env]")
{
   json j = {
      {"name", "Env"},
      {"os", "Windows"},
      {"osVersion", "11"}
   };

   EXPECT_SINGLE_ERROR(ValidateSoftwareEnvironment(j), "driverFamily");
}

TEST_CASE("ValidateSoftwareEnvironment - empty required fields", "[validation][env]")
{
   json j = {
      {"name", ""},
      {"os", ""},
      {"driverFamily", ""}
   };

   auto errors = ValidateSoftwareEnvironment(j);
   REQUIRE(errors.size() == 3);
}

TEST_CASE("ValidateSoftwareEnvironment - invalid types", "[validation][env]")
{
   json j = {
      {"name", 123},
      {"os", true},
      {"driverFamily", 999}
   };

   auto errors = ValidateSoftwareEnvironment(j);
   REQUIRE(errors.size() == 3);
}

TEST_CASE("ValidateSoftwareEnvironment - optional osVersion allowed", "[validation][env]")
{
   json j = {
      {"name", "Env"},
      {"os", "Windows"},
      {"driverFamily", "NVIDIA"},
      {"osVersion", 123} // invalid type but optional → should still error
   };

   EXPECT_SINGLE_ERROR(ValidateSoftwareEnvironment(j), "osVersion");
}

TEST_CASE("ValidateSoftwareEnvironment - unknown fields are just ignored", "[validation][env]")
{
   json j = {
      {"name", "Env"},
      {"os", "Windows"},
      {"driverFamily", "NVIDIA"},
      {"osVersion", "123"},
      {"myUnusedField", 456}
   };

   REQUIRE(ValidateSoftwareEnvironment(j).empty());
}

