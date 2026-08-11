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
   BiosInfo GetBios() override;

   VideoDriverInfo GetVideoDriver() override;
   OsInfo GetOs() override;

private:
   std::string GetCpuBrandString() const;
   int GetCpuCoreCount() const;
   std::string GetInfoFileContent(const std::string& file) const;

   std::vector<std::string> GetGlxInfoOutput() const;
   std::string FindGpuVendor(const std::vector<std::string>& glxInfoData) const;
   std::string FindGpuName(const std::vector<std::string>& glxInfoData) const;
   int FindVramQuantity(const std::vector<std::string>& glxInfoData) const;

   int FindRamFrequencyFromDmiDecode() const;
   std::string FindFromTag(const std::string& tag, const std::string& line) const;

   std::vector<int> GetNvidiaSmiValues() const;
   int GetGpuMinFrequencyFromSysfs() const;
   std::string GetBiosFileInfo(const std::string& filename) const;
   std::string GetProperty(const std::string& propertyName, const std::vector<std::string>& lines) const;

   std::string GetDriverInfo() const;
};

#endif // LINUXSYSTEMINFO_H
