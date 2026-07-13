#include "Utils.h"

using json = nlohmann::json;
using string = std::string;

bool Utils::ContainsNested(const json& node, const std::vector<string>& properties)
{
    const json* currentNode = &node;
    for (const auto& prop : properties)
    {
        if (!currentNode->contains(prop))
            return false;
        currentNode = &currentNode->at(prop);
    }
    return true;
}

string Utils::GetNested(const nlohmann::json& node, const std::vector<string>& properties)
{
    const json* currentNode = &node;
    for (const auto& prop : properties)
    {
        if (!currentNode->contains(prop))
            return "";
        currentNode = &currentNode->at(prop);
    }
    return currentNode->get<string>();
}
