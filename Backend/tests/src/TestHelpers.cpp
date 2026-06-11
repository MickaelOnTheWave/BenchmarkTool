#include "TestHelpers.h"

void EXPECT_SINGLE_ERROR(const std::vector<std::string>& errors,
                         const std::string& expectedSubstring1,
                         const std::string& expectedSubstring2)
{
   REQUIRE(errors.size() == 1);

   const auto& msg = errors[0];

   REQUIRE(msg.find(expectedSubstring1) != std::string::npos);

   if (!expectedSubstring2.empty())
      REQUIRE(msg.find(expectedSubstring2) != std::string::npos);
}
