#include "ThreeDMarkImporter.h"

#include "tinyxml2.h"
#include "Utils.h"
#include "ZipArchive.h"

using json = nlohmann::json;

namespace
{
   tinyxml2::XMLElement* FindDirectChildByName(tinyxml2::XMLNode* node, const char* name)
   {
      if (!node)
         return nullptr;

      for (auto* elem = node->FirstChildElement();
           elem;
           elem = elem->NextSiblingElement())
      {
         if (strcmp(elem->Name(), name) == 0)
            return elem;
      }

      return nullptr;
   };

   tinyxml2::XMLElement* FindChildBySubtag(tinyxml2::XMLNode* parent, const char* name, const char* value)
   {
      if (!parent)
         return nullptr;

      for (auto* elem = parent->FirstChildElement();
           elem;
           elem = elem->NextSiblingElement())
      {
         auto* elemProperty = FindDirectChildByName(elem, name);
         if (!elemProperty)
            return nullptr;

         if (strcmp(elemProperty->Name(), name) == 0 && strcmp(elemProperty->GetText(), value) == 0)
            return elem;
      }

      return nullptr;
   };

   tinyxml2::XMLElement* FindFirstElementByName(tinyxml2::XMLNode* node, const char* name)
   {
      if (!node)
         return nullptr;

      for (auto* elem = node->FirstChildElement();
           elem;
           elem = elem->NextSiblingElement())
      {
         if (strcmp(elem->Name(), name) == 0)
            return elem;

         if (auto* found = FindFirstElementByName(elem, name))
            return found;
      }

      return nullptr;
   };

   std::string GetNodeValue(tinyxml2::XMLNode* parent, const char* tag)
   {
      auto* elem = FindFirstElementByName(parent, tag);

      if (!elem || !elem->GetText())
         return {};

      return elem->GetText();
   };

   std::string GetNodeValueByNameProperty(tinyxml2::XMLNode* propertyListNode, const std::string& targetName)
   {
       auto targetPropertyNode = FindChildBySubtag(propertyListNode, "name", targetName.c_str());
       return GetNodeValue(targetPropertyNode, "value");
   }
}

void FillMachineInformation(tinyxml2::XMLDocument& doc, json& result)
{
   auto cpuNode = FindFirstElementByName(&doc, "Processor");
   if (cpuNode)
   {
      result["machine"]["cpu"]["name"] = GetNodeValue(cpuNode, "Processor_Name");
      result["machine"]["cpu"]["frequency"] = GetNodeValue(cpuNode, "Stock_Clock_Frequency");
      result["machine"]["cpu"]["core_count"] = GetNodeValue(cpuNode, "Core_Count");
      result["machine"]["cpu"]["thread_count"] = GetNodeValue(cpuNode, "Thread_Count");
   }

   auto gpuSectionNode = FindFirstElementByName(&doc, "GPUs");
   if (gpuSectionNode)
   {
      // We parsed before TestInfo, so we take from there which GPU was used.
      const std::string usedGpuName = result["test"]["usedGpu"];
      auto gpuNode = FindChildBySubtag(gpuSectionNode, "CARD_NAME", usedGpuName.c_str());
      if (gpuNode)
      {
         result["machine"]["gpu"]["name"] = GetNodeValue(gpuNode, "CARD_NAME");
         result["machine"]["gpu"]["frequency"] = GetNodeValue(gpuNode, "CLOCK_GPU");
         result["machine"]["gpu"]["ramQuantityGb"] = GetNodeValue(gpuNode, "MEM_SIZE");
         result["machine"]["gpu"]["memoryFrequency"] = GetNodeValue(gpuNode, "CLOCK_MEM");
         result["machine"]["gpu"]["memoryType"] = GetNodeValue(gpuNode, "MEM_TYPE");
      }
   }

   auto motherboardNode = FindFirstElementByName(&doc, "Motherboard");
   if (motherboardNode)
   {
      result["machine"]["motherboard"]["model"] = GetNodeValue(motherboardNode, "Mainboard_Model");
      result["machine"]["motherboard"]["northbridge"] = GetNodeValue(motherboardNode, "North_Bridge_Model");
      result["machine"]["motherboard"]["southbridge"] = GetNodeValue(motherboardNode, "South_Bridge_Model");
      result["machine"]["motherboard"]["memoryslots"] = GetNodeValue(motherboardNode, "Memory_Slot_Count");
   }

   auto infoNode = FindFirstElementByName(&doc, "Direct_Query_Info");
   if (infoNode)
   {
      // Must put ram in Gb, 3DMark stores it in Mb.
      const std::string ramStr = GetNodeValue(infoNode, "Memory");
      result["machine"]["ram"] = std::stoi(ramStr) / 1024;
   }
}

void FillSoftwareInformation(tinyxml2::XMLDocument& doc, json& result)
{
   auto infoNode = FindFirstElementByName(&doc, "Direct_Query_Info");
   if (infoNode)
   {
      result["softwareenvironment"]["os"] = GetNodeValue(infoNode, "OS");
   }

   auto motherboardNode = FindFirstElementByName(&doc, "Motherboard");
   if (motherboardNode)
   {
      result["softwareenvironment"]["bios"]["version"] = GetNodeValue(motherboardNode, "BIOS_Version");
      result["softwareenvironment"]["bios"]["date"] = GetNodeValue(motherboardNode, "BIOS_Date");
   }

   auto directxNode = FindFirstElementByName(&doc, "DirectX_Info");

   const std::string selectedGpu = Utils::GetNested(result, {"machine", "gpu", "name"});
   if (directxNode && selectedGpu != "")
   {
      auto directDrawNode = FindFirstElementByName(directxNode, "DirectDraw_Info");
      if (directDrawNode)
      {
         auto displayDevicesNode = FindFirstElementByName(directDrawNode, "Display_Devices");
         if (displayDevicesNode)
         {
            auto gpuNameNode = FindChildBySubtag(displayDevicesNode, "Description", selectedGpu.c_str());
            if (gpuNameNode)
            {
               result["softwareenvironment"]["driverversion"] = GetNodeValue(gpuNameNode, "Driver_Version");
            }
         }
      }
   }
}

json ThreeDMarkImporter::Import(const ByteBuffer& fileData)
{
   json result;

   ZipArchive archive(fileData);
   if (!archive.IsValid())
   {
      result["error"] = "Invalid ZIP archive";
      return result;
   }

   const ByteBuffer testXml = archive.ExtractFile("Arielle.xml");
   if (!testXml.empty())
      ParseTestInfoXml(result, testXml);

   const ByteBuffer resultXml = archive.ExtractFile("Result.xml");
   if (!resultXml.empty())
      ParseResultXml(result, resultXml);

   const ByteBuffer siXml = archive.ExtractFile("SI.xml");
   if (!siXml.empty())
      ParseSystemInfoXml(result, siXml);

   return result;
}

void ThreeDMarkImporter::ParseResultXml(json& result, const ByteBuffer& data)
{
   tinyxml2::XMLDocument doc;

   const std::string xml = ByteBufferToString(data);

   if (doc.Parse(xml.c_str(), xml.size()) != tinyxml2::XML_SUCCESS)
   {
      result["benchmarkRun"]["error"] = "Failed to parse Result.xml";
       return;
   }

   auto* root = doc.FirstChildElement("benchmark");
   if (!root)
   {
      result["benchmarkRun"]["error"] = "Missing <benchmark> root";
       return;
   }

   auto* results = root->FirstChildElement("results");
   if (!results)
   {
      result["benchmarkRun"]["error"] = "Missing <results>";
      return;
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
      result["benchmarkRun"]["error"] = "No aggregate result (passIndex -1) found";
       return;
   }

   // benchmarkRunId
   const auto* runIdElem = bestResult->FirstChildElement("benchmarkRunId");
   if (runIdElem && runIdElem->GetText())
      result["benchmarkRun"]["origin"]["runExternalId"] = runIdElem->GetText();

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

   result["benchmarkRun"]["result"]["score"] = score;
}

void ThreeDMarkImporter::ParseSystemInfoXml(json& result, const ByteBuffer& data)
{
   tinyxml2::XMLDocument doc;

   const std::string xml(reinterpret_cast<const char*>(data.data()),data.size());
   if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS)
      return;

   FillMachineInformation(doc, result);
   FillSoftwareInformation(doc, result);

   auto systemInfoNode = FindFirstElementByName(&doc, "System_Info");
   if (systemInfoNode)
   {
      result["machine"]["origin"]["machine-guid"] = GetNodeValue(systemInfoNode, "Guid");
   }
}

void ThreeDMarkImporter::ParseTestInfoXml(json& result, const ByteBuffer& data)
{
   tinyxml2::XMLDocument doc;

   const std::string xml(reinterpret_cast<const char*>(data.data()),data.size());
   if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS)
      return;

   auto appInfoNode = FindFirstElementByName(&doc, "application_info");
   if (appInfoNode)
   {
       auto startTimeNode = GetNodeValueByNameProperty(appInfoNode, "benchmark_start_time");
       result["benchmarkRun"]["time"] = startTimeNode;
   }

   auto testInfoNode = FindFirstElementByName(&doc, "test_info");
   if (testInfoNode)
   {
      auto testNode = FindFirstElementByName(&doc, "benchmark_test");
      if (testNode)
      {
         std::string rawTestName = testNode->Attribute("name");
         rawTestName.pop_back();
         result["test"]["name"] = rawTestName;
      }
   }

   auto settingsNode = FindDirectChildByName(doc.RootElement(), "settings");
   auto FindSettingValue = [settingsNode](const char* settingName)
   {
       return GetNodeValueByNameProperty(settingsNode, settingName);
   };

   result["test"]["usedGpu"] = FindSettingValue("adapter_name");
   result["test"]["textureFilterMode"] = FindSettingValue("texture_filtering_mode");
   result["test"]["resolution"] = FindSettingValue("rendering_resolution");
}

std::string ThreeDMarkImporter::ByteBufferToString(const ByteBuffer& data)
{
   return std::string(data.begin(), data.end());
}
