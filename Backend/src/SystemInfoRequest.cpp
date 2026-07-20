#include "SystemInfoRequest.h"

#include "LinuxSystemInfo.h"

using json = nlohmann::json;

nlohmann::json SystemInfoRequest::CreateJsonResponse()
{
   LinuxSystemInfo systemInfo;

   json j;

   const auto cpuInfo = systemInfo.GetCpu();
   SetFieldValues(j, "cpu", cpuInfo);
   j["cpu"]["coreCount"] = cpuInfo.coreCount;

   const auto ram = systemInfo.GetRam();
   SetFieldValues(j, "ram", ram);

   const auto gpuInfo = systemInfo.GetGpu();
   SetFieldValues(j, "gpu", gpuInfo);
   SetFieldValues(j["gpu"], "vram", gpuInfo.vram);

   auto motherboard = systemInfo.GetMotherboard();
   j["motherboard"]["name"] = motherboard;

   auto bios = systemInfo.GetBios();
   SetFieldValues(j["motherboard"], "bios", bios);

   auto videoDriver = systemInfo.GetVideoDriver();
   j["gpu"]["driver"]["type"] = videoDriver.type;
   j["gpu"]["driver"]["version"] = videoDriver.version;

   auto os = systemInfo.GetOs();
   j["os"]["name"] = os.name;
   j["os"]["version"] = os.version;

   j["status"] = "ok";
   return j;
}

void SystemInfoRequest::SetFieldValues(nlohmann::json &j, const std::string &propertyFamily, const ProcessingUnitInfo& info)
{
   j[propertyFamily]["name"] = info.name;
   j[propertyFamily]["minFrequencyMhz"] = info.minFrequencyMhz;
   j[propertyFamily]["maxFrequencyMhz"] = info.maxFrequencyMhz;
   j[propertyFamily]["currentFrequencyMhz"] = info.currentFrequencyMhz;
}

void SystemInfoRequest::SetFieldValues(nlohmann::json &j, const std::string &propertyFamily, const MemoryInfo& info)
{
   j[propertyFamily]["quantityMb"] = info.quantityMb;
   j[propertyFamily]["frequencyMhz"] = info.frequencyMhz;
}

void SystemInfoRequest::SetFieldValues(nlohmann::json &j, const std::string &propertyFamily, const BiosInfo& info)
{
   j[propertyFamily]["vendor"] = info.vendor;
   j[propertyFamily]["version"] = info.version;
   j[propertyFamily]["date"] = info.date;
}
