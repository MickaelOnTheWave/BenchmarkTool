#ifndef LINUXSYSTEMINFO_H
#define LINUXSYSTEMINFO_H

#include "ISystemInfo.h"

#include <vector>

class LinuxSystemInfo : public ISystemInfo
{
public:
   LinuxSystemInfo() = default;

   CpuInfo GetCpu() override;
   GpuInfo GetGpu() override;
   MemoryInfo GetRam() override;
   std::string GetMotherboard() override;
   std::string GetBiosVersion() override;

   std::string GetOsName() override;
   std::string GetOsVersion() override;
   std::string GetVideoDriver() override;

private:
   std::string GetCpuBrandString() const;
   int GetCpuCoreCount() const;
   std::string GetInfoFileContent(const std::string& file) const;

   std::vector<std::string> GetGlxInfoOutput() const;
   std::string FindGpuVendor(const std::vector<std::string>& glxInfoData) const;
   std::string FindGpuName(const std::vector<std::string>& glxInfoData) const;
   int FindVramQuantity(const std::vector<std::string>& glxInfoData) const;

   std::vector<int> GetNvidiaSmiValues() const;
};

#endif // LINUXSYSTEMINFO_H
