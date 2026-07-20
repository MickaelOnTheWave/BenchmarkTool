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

struct BiosInfo
{
   std::string vendor;
   std::string version;
   std::string date;
};

struct VideoDriverInfo
{
   std::string type;
   std::string version;
};

struct OsInfo
{
   std::string name;
   std::string version;
};


class ISystemInfo
{
public:
   virtual CpuInfo GetCpu() = 0;
   virtual GpuInfo GetGpu() = 0;
   virtual MemoryInfo GetRam() = 0;
   virtual std::string GetMotherboard() = 0;
   virtual BiosInfo GetBios() = 0;

   virtual VideoDriverInfo GetVideoDriver() = 0;
   virtual OsInfo GetOs() = 0;
};

#endif // ISYSTEMINFO_H
