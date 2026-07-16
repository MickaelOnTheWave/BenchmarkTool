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
/*
   const auto vram = systemInfo.GetVram();
   SetFieldValues(j, "vram", vram);

   systemInfo.GetMotherboard();

   systemInfo.GetBiosVersion();

   systemInfo.GetVideoDriver();

   systemInfo.GetOsName();

   systemInfo.GetOsVersion()*/

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
