#ifndef THREEDMARKIMPORTER_H
#define THREEDMARKIMPORTER_H

#include "AbstractImporter.h"

class ThreeDMarkImporter : public AbstractImporter
{
public:
   nlohmann::json Import(const ByteBuffer& fileData) override;

private:
   nlohmann::json ParseResultXml(const ByteBuffer& data);
   nlohmann::json ParseSystemInfoXml(const ByteBuffer& data);
   nlohmann::json ParseMonitoringCsv(const ByteBuffer& data);

   static std::string ByteBufferToString(const ByteBuffer& data);
};

#endif // THREEDMARKIMPORTER_H
