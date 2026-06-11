#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include "EntityValidators.h"
#include "TestHelpers.h"

using json = nlohmann::json;

TEST_CASE("ValidateMachine - valid machine", "[validation][machine]")
{
   json j = {
      {"name", "Workstation Alpha"},
      {"cpu", "Intel i9-14900K"},
      {"gpu", "RTX 4090"},
      {"ramGb", 64},
      {"motherboard", "ASUS Z790"}
   };

   auto errors = ValidateMachine(j);
   REQUIRE(errors.empty());
}

TEST_CASE("ValidateMachine - missing name", "[machine]")
{
   json j = {
      {"cpu", "Intel i9"},
      {"gpu", "RTX 4090"},
      {"ramGb", 64},
      {"motherboard", "ASUS Z790"}
   };

   auto errors = ValidateMachine(j);
   EXPECT_SINGLE_ERROR(errors, "name", "required");
}

TEST_CASE("ValidateMachine - missing cpu", "[machine]")
{
   json j = {
      {"name", "Workstation"},
      {"gpu", "RTX 4090"},
      {"ramGb", 64},
      {"motherboard", "ASUS Z790"}
   };

   auto errors = ValidateMachine(j);
   EXPECT_SINGLE_ERROR(errors, "cpu", "required");
}

TEST_CASE("ValidateMachine - missing gpu", "[machine]")
{
   json j = {
      {"name", "Workstation"},
      {"cpu", "Intel i9"},
      {"ramGb", 64},
      {"motherboard", "ASUS Z790"}
   };

   auto errors = ValidateMachine(j);
   EXPECT_SINGLE_ERROR(errors, "gpu", "required");
}

TEST_CASE("ValidateMachine - missing ramGb", "[machine]")
{
   json j = {
      {"name", "Workstation"},
      {"cpu", "Intel i9"},
      {"gpu", "RTX 4090"},
      {"motherboard", "ASUS Z790"}
   };

   auto errors = ValidateMachine(j);
   EXPECT_SINGLE_ERROR(errors, "ram", "required");
}

TEST_CASE("ValidateMachine - invalid RAM", "[validation][machine]")
{
   json j = {
      {"name", "Workstation Alpha"},
      {"cpu", "Intel i9"},
      {"gpu", "RTX 4090"},
      {"ramGb", -16},
      {"motherboard", "ASUS Z790"}
   };

   auto errors = ValidateMachine(j);
   EXPECT_SINGLE_ERROR(errors, "ram", "> 0");
}

TEST_CASE("ValidateMachine - missing motherboard", "[machine]")
{
   json j = {
      {"name", "Workstation"},
      {"cpu", "Intel i9"},
      {"gpu", "RTX 4090"},
      {"ramGb", 64}
   };

   auto errors = ValidateMachine(j);
   EXPECT_SINGLE_ERROR(errors, "motherboard", "required");
}

TEST_CASE("ValidateMachine - wrong types", "[validation][machine]")
{
   json j = {
      {"name", 123},
      {"cpu", true},
      {"gpu", nullptr},
      {"ramGb", "64GB"},
      {"motherboard", 999}
   };

   auto errors = ValidateMachine(j);
   REQUIRE(errors.size() == 5);
}
