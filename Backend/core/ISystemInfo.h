#ifndef ISYSTEMINFO_H
#define ISYSTEMINFO_H

#include <string>

struct ProcessingUnitInfo
{
   std::string name;
   int minFrequencyMhz;
   int maxFrequencyMhz;
   int currentFrequencyMhz;
};

struct CpuInfo : public ProcessingUnitInfo
{
   int coreCount;
};


struct MemoryInfo
{
   int quantityMb;
   int frequencyMhz;
};

class ISystemInfo
{
public:
   virtual CpuInfo GetCpu() = 0;
   virtual ProcessingUnitInfo GetGpu() = 0;
   virtual MemoryInfo GetRam() = 0;
   virtual MemoryInfo GetVram() = 0;
   virtual std::string GetMotherboard() = 0;
   virtual std::string GetBiosVersion() = 0;

   virtual std::string GetOsName() = 0;
   virtual std::string GetOsVersion() = 0;
   virtual std::string GetVideoDriver() = 0;
};

#endif // ISYSTEMINFO_H
