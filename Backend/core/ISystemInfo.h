#ifndef ISYSTEMINFO_H
#define ISYSTEMINFO_H

#include <string>

struct ProcessingUnitInfo
{
   std::string name;
   std::string vendor;
   int minFrequencyMhz;
   int maxFrequencyMhz;
   int currentFrequencyMhz;
};

struct MemoryInfo
{
   int quantityMb;
   int frequencyMhz;
};

struct CpuInfo : public ProcessingUnitInfo
{
   int coreCount;
};

struct GpuInfo : public ProcessingUnitInfo
{
   MemoryInfo vram;
};


class ISystemInfo
{
public:
   virtual CpuInfo GetCpu() = 0;
   virtual GpuInfo GetGpu() = 0;
   virtual MemoryInfo GetRam() = 0;
   virtual std::string GetMotherboard() = 0;
   virtual std::string GetBiosVersion() = 0;

   virtual std::string GetOsName() = 0;
   virtual std::string GetOsVersion() = 0;
   virtual std::string GetVideoDriver() = 0;
};

#endif // ISYSTEMINFO_H
