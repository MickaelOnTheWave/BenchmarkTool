#include <catch2/catch_test_macros.hpp>

//#include "Validation.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

TEST_CASE("Valid machine passes validation", "[machine]")
{
   json machine =
      {
         {"name", "Workstation Alpha"},
         {"cpu", "Intel Core i9-14900K"},
         {"gpu", "RTX 5090"},
         {"ramGb", 64},
         {"motherboard", "ASUS Z790"}
      };

   //auto error = ValidateMachine(machine);
   //REQUIRE_FALSE(error.has_value());
   REQUIRE(false);
}

TEST_CASE("Machine with empty name is rejected", "[machine]")
{
   json machine =
      {
         {"name", ""},
         {"cpu", "Intel Core i9-14900K"},
         {"gpu", "RTX 5090"},
         {"ramGb", 64},
         {"motherboard", "ASUS Z790"}
      };

   //auto error = ValidateMachine(machine);
   //REQUIRE(error.has_value());
   REQUIRE(false);
}
