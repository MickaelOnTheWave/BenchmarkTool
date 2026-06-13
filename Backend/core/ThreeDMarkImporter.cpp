#include "ThreeDMarkImporter.h"

#include "tinyxml2.h"
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
   json result;
   tinyxml2::XMLDocument doc;

   const std::string xml = ByteBufferToString(data);

   if (doc.Parse(xml.c_str(), xml.size()) != tinyxml2::XML_SUCCESS)
   {
      result["error"] = "Failed to parse Result.xml";
      return result;
   }

   auto* root = doc.FirstChildElement("benchmark");
   if (!root)
   {
      result["error"] = "Missing <benchmark> root";
      return result;
   }

   auto* results = root->FirstChildElement("results");
   if (!results)
   {
      result["error"] = "Missing <results>";
      return result;
   }

   const tinyxml2::XMLElement* bestResult = nullptr;

   for (auto* r = results->FirstChildElement("result"); r; r = r->NextSiblingElement("result"))
   {
      const char* passIndexText = r->FirstChildElement("passIndex") ? r->FirstChildElement("passIndex")->GetText() : nullptr;

      if (passIndexText && std::string(passIndexText) == "-1")
      {
         bestResult = r;
         break;
      }
   }

   if (!bestResult)
   {
      result["error"] = "No aggregate result (passIndex -1) found";
      return result;
   }

   // benchmarkRunId
   const auto* runIdElem = bestResult->FirstChildElement("benchmarkRunId");
   if (runIdElem && runIdElem->GetText())
      result["origin"]["runExternalId"] = runIdElem->GetText();

   // score (first element ending with "Score")
   int score = 0;

   for (auto* e = bestResult->FirstChildElement(); e; e = e->NextSiblingElement())
   {
      std::string name = e->Name();

      if (name.size() >= 5 && name.find("Score") != std::string::npos &&
          name.find("ScoreForPass") == std::string::npos)
      {
         const char* txt = e->GetText();
         if (txt)
         {
            score = std::atoi(txt);
            break;
         }
      }
   }

   result["result"]["score"] = score;
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
