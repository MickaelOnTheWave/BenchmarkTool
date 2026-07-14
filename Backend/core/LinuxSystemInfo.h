#ifndef LINUXSYSTEMINFO_H
#define LINUXSYSTEMINFO_H

#include "ISystemInfo.h"

class LinuxSystemInfo : public ISystemInfo
{
public:
   LinuxSystemInfo() = default;

   CpuInfo GetCpu() override;
   ProcessingUnitInfo GetGpu() override;
   MemoryInfo GetRam() override;
   MemoryInfo GetVram() override;
   std::string GetMotherboard() override;
   std::string GetBiosVersion() override;

   std::string GetOsName() override;
   std::string GetOsVersion() override;
   std::string GetVideoDriver() override;

private:
   std::string GetCpuBrandString() const;
   int GetCpuCoreCount() const;
   std::string GetInfoFileContent(const std::string& file) const;
};

#endif // LINUXSYSTEMINFO_H
