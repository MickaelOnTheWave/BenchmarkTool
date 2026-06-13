#ifndef ABSTRACTIMPORTER_H
#define ABSTRACTIMPORTER_H

#include <nlohmann/json.hpp>

using ByteBuffer = std::vector<uint8_t>;

class AbstractImporter
{
public:
   virtual ~AbstractImporter() = default;

   virtual nlohmann::json Import(const ByteBuffer& fileData) = 0;
};

#endif // ABSTRACTIMPORTER_H
