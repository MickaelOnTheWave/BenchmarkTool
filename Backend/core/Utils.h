#ifndef UTILS_H
#define UTILS_H

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Utils
{
    bool ContainsNested(const nlohmann::json& node, const std::vector<std::string>& properties);
    std::string GetNested(const nlohmann::json& node, const std::vector<std::string>& properties);
}

#endif // UTILS_H
