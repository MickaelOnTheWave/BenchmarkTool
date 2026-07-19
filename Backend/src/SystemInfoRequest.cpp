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
/*
   auto motherboard = systemInfo.GetMotherboard();

   auto biosVersion = systemInfo.GetBiosVersion();

   auto videoDriver = systemInfo.GetVideoDriver();

   auto osName = systemInfo.GetOsName();
*/
   //auto osVersion = systemInfo.GetOsVersion();

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
