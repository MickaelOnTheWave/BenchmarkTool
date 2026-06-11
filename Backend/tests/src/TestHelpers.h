#ifndef TESTHELPERS_H
#define TESTHELPERS_H

#include <vector>
#include <string>
#include <catch2/catch_test_macros.hpp>

void EXPECT_SINGLE_ERROR(const std::vector<std::string>& errors,
                         const std::string& expectedSubstring1,
                         const std::string& expectedSubstring2 = "");

#endif // TESTHELPERS_H
