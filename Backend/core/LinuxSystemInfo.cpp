#include "LinuxSystemInfo.h"

#include <cpuid.h>
#include <cstring>
#include <sys/sysinfo.h>

#include <fstream>
#include <sstream>

using namespace std;

namespace
{
   int InMhz(const std::string& value)
   {
      try
      {
         return stoi(value) / 1000;
      }
      catch (...)
      {
         return -1;
      }
   }
}

CpuInfo LinuxSystemInfo::GetCpu()
{
   CpuInfo info;
   //info.name = GetInfoFileContent("/proc/cpuinfo");

   info.name = GetCpuBrandString();
   info.coreCount = GetCpuCoreCount();


   info.minFrequencyMhz = InMhz(GetInfoFileContent("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq"));
   info.maxFrequencyMhz = InMhz(GetInfoFileContent("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"));
   info.currentFrequencyMhz = InMhz(GetInfoFileContent("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"));
   return info;
}

ProcessingUnitInfo LinuxSystemInfo::GetGpu()
{
   return ProcessingUnitInfo();
}

MemoryInfo LinuxSystemInfo::GetRam()
{
   MemoryInfo info;
   struct sysinfo hwInfo;
   if (sysinfo(&hwInfo) == 0)
   {
      static const int megaByteInB = 1024 * 1024;
      info.quantityMb = hwInfo.totalram * hwInfo.mem_unit / megaByteInB;
   }
   info.frequencyMhz = -1;

   return info;
}

MemoryInfo LinuxSystemInfo::GetVram()
{

}

std::string LinuxSystemInfo::GetMotherboard()
{

}

std::string LinuxSystemInfo::GetBiosVersion()
{

}

std::string LinuxSystemInfo::GetOsName()
{

}

std::string LinuxSystemInfo::GetOsVersion()
{

}

std::string LinuxSystemInfo::GetVideoDriver()
{

}

int LinuxSystemInfo::GetCpuCoreCount() const
{
   unsigned int eax, ebx, ecx, edx;

   __get_cpuid(1, &eax, &ebx, &ecx, &edx);

   const unsigned logical = (ebx >> 16) & 0xff;
   return static_cast<int>(logical);
}

string LinuxSystemInfo::GetCpuBrandString() const
{
   unsigned int regs[4];
   char brand[49];

   for (unsigned int i = 0; i < 3; ++i)
   {
      __get_cpuid(0x80000002 + i,
                  &regs[0], &regs[1], &regs[2], &regs[3]);

      memcpy(brand + i * 16, regs, 16);
   }

   return string(brand);
}

string LinuxSystemInfo::GetInfoFileContent(const std::string& file) const
{
   ifstream infoFile(file);
   if (infoFile.is_open())
   {
      ostringstream stream;
      stream << infoFile.rdbuf();
      return stream.str();
   }
   return "";
}
