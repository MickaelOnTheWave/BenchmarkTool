#include "ThreeDMarkImporter.h"

#include "ZipArchive.h"

using json = nlohmann::json;

json ThreeDMarkImporter::Import(const ByteBuffer& fileData)
{
   json result;

   ZipArchive archive(fileData);
   if (!archive.IsValid())
   {
      result["error"] = "Invalid ZIP archive";
      return result;
   }

   const ByteBuffer resultXml = archive.ExtractFile("Result.xml");
   const ByteBuffer siXml = archive.ExtractFile("SI.xml");
   const ByteBuffer monitoringCsv = archive.ExtractFile("Monitoring.csv");

   if (!resultXml.empty())
      result["benchmarkRun"] = ParseResultXml(resultXml);

   if (!siXml.empty())
      result["systemInfo"] = ParseSystemInfoXml(siXml);

   if (!monitoringCsv.empty())
      result["monitoring"] = ParseMonitoringCsv(monitoringCsv);

   return result;
}

json ThreeDMarkImporter::ParseResultXml(const ByteBuffer& data)
{
   (void)data;

   json result;
   return result;
}

json ThreeDMarkImporter::ParseSystemInfoXml(const ByteBuffer& data)
{
   (void)data;

   json result;
   return result;
}

json ThreeDMarkImporter::ParseMonitoringCsv(const ByteBuffer& data)
{
   (void)data;

   json result;
   return result;
}

std::string ThreeDMarkImporter::ByteBufferToString(const ByteBuffer& data)
{
   return std::string(data.begin(), data.end());
}
