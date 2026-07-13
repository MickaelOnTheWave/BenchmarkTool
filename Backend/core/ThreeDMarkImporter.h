#ifndef THREEDMARKIMPORTER_H
#define THREEDMARKIMPORTER_H

#include "AbstractImporter.h"

class ThreeDMarkImporter : public AbstractImporter
{
public:
   nlohmann::json Import(const ByteBuffer& fileData) override;

private:
   void ParseResultXml(nlohmann::json& result, const ByteBuffer& data);
   void ParseTestInfoXml(nlohmann::json& result, const ByteBuffer& data);
   void ParseSystemInfoXml(nlohmann::json& result, const ByteBuffer& data);

   static std::string ByteBufferToString(const ByteBuffer& data);
};

#endif // THREEDMARKIMPORTER_H
