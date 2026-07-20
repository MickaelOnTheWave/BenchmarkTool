#include "LinuxSystemInfo.h"

#include <cpuid.h>
#include <cstring>
#include <sys/sysinfo.h>

#include <fstream>
#include <sstream>

#include "stringtools.h"
#include "tools.h"

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
   info.name = GetCpuBrandString();
   info.coreCount = GetCpuCoreCount();

   info.minFrequencyMhz = InMhz(GetInfoFileContent("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq"));
   info.maxFrequencyMhz = InMhz(GetInfoFileContent("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"));
   info.currentFrequencyMhz = InMhz(GetInfoFileContent("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"));
   return info;
}

GpuInfo LinuxSystemInfo::GetGpu()
{
   GpuInfo info;

   const vector<string> glxInfoData = GetGlxInfoOutput();
   info.name = FindGpuName(glxInfoData);
   info.vendor = FindGpuVendor(glxInfoData);
   info.vram.quantityMb = FindVramQuantity(glxInfoData);

   info.minFrequencyMhz = -1;
   if (info.vendor == "NVIDIA Corporation")
   {
      const vector<int> nvidiaSmiData = GetNvidiaSmiValues();
      info.maxFrequencyMhz = nvidiaSmiData[0];
      info.currentFrequencyMhz = nvidiaSmiData[1];
      info.vram.frequencyMhz = nvidiaSmiData[2];
   }
   else
   {
      info.maxFrequencyMhz = -1;
      info.currentFrequencyMhz = -1;
      info.vram.frequencyMhz = -1;
   }

   return info;
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

std::string LinuxSystemInfo::GetMotherboard()
{
   std::ifstream boardName("/sys/class/dmi/id/board_name");
   if (boardName.is_open())
   {
      std::string line;
      std::getline(boardName, line);
      return line;
   }
   return "Unknown Motherboard";
}

BiosInfo LinuxSystemInfo::GetBios()
{
   BiosInfo info;
   info.vendor = GetBiosFileInfo("bios_vendor");
   info.version = GetBiosFileInfo("bios_release");
   info.date = GetBiosFileInfo("bios_date");
   return info;
}

VideoDriverInfo LinuxSystemInfo::GetVideoDriver()
{
   VideoDriverInfo info;
   return info;
}

OsInfo LinuxSystemInfo::GetOs()
{
   vector<string> fileLines;
   StringTools::Tokenize(GetInfoFileContent("/etc/os-release"), '\n', fileLines);

   OsInfo info;
   info.name = GetProperty("NAME", fileLines);
   info.version = GetProperty("VERSION_ID", fileLines);
   return info;
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

std::vector<std::string> LinuxSystemInfo::GetGlxInfoOutput() const
{
   vector<string> lines;
   std::wstring output;
   const int returnCode = Tools::RunExternalCommandToBuffer(L"glxinfo", output);
   if (returnCode == 0)
      StringTools::Tokenize(StringTools::UnicodeToUtf8(output), '\n', lines);
   return lines;
}

std::string LinuxSystemInfo::FindGpuVendor(const std::vector<std::string>& glxInfoData) const
{
   static const string rendererTag = "OpenGL vendor string: ";
   for (const auto& line : glxInfoData)
   {
      const size_t pos = line.find(rendererTag);
      if (pos == 0)
         return line.substr(rendererTag.size());
   }
   return "Unknown";
}

std::string LinuxSystemInfo::FindGpuName(const std::vector<std::string>& glxInfoData) const
{
   static const string rendererTag = "OpenGL renderer string: ";
   for (const auto& line : glxInfoData)
   {
      const size_t pos = line.find(rendererTag);
      if (pos == 0)
      {
         const string baseName = line.substr(rendererTag.size());
         vector<string> nameAndDetails;
         StringTools::Tokenize(baseName, '/', nameAndDetails);
         return nameAndDetails.front();
      }
   }
   return "Unknown GPU";
}

int LinuxSystemInfo::FindVramQuantity(const std::vector<std::string> &glxInfoData) const
{
   static const string rendererTag = "Total available memory: ";
   for (const auto& line : glxInfoData)
   {
      const size_t pos = line.find(rendererTag);
      if (pos != string::npos)
      {
         string data = line.substr(pos + rendererTag.size());
         const size_t mbPos = data.find(" MB");
         data = data.substr(0, mbPos);
         return std::atoi(data.c_str());
      }
   }
   return -1;
}

std::vector<int> LinuxSystemInfo::GetNvidiaSmiValues() const
{
   std::wstring output;
   const int returnCode = Tools::RunExternalCommandToBuffer(L"nvidia-smi --query-gpu=clocks.max.graphics,clocks.current.graphics,clocks.max.memory,clocks.current.memory --format=csv", output);
   if (returnCode == 0)
   {
      vector<string> lines;
      StringTools::Tokenize(StringTools::UnicodeToUtf8(output), '\n', lines);

      vector<string> values;
      StringTools::Tokenize(lines.back(), ',', values);

      vector<int> outputValues;
      static const string endTag = " Mhz";
      for (auto& value : values)
      {
         const size_t startPos = value.find_first_not_of(' ');
         const size_t endPos = value.find(endTag);
         const int output = std::atoi(value.substr(startPos, endPos).c_str());
         outputValues.push_back(output);
      }
      return outputValues;
   }
   return {-1, -1, -1, -1};

}

string LinuxSystemInfo::GetBiosFileInfo(const std::string &filename) const
{
   const string fullFilename = "/sys/class/dmi/id/" + filename;
   std::ifstream biosFile(fullFilename);
   if (biosFile.is_open())
   {
      std::string line;
      std::getline(biosFile, line);
      return line;
   }
   return "Unknown";
}

string LinuxSystemInfo::GetProperty(const std::string &propertyName, const std::vector<std::string> &lines) const
{
   const string tagToFind = propertyName + "=";
   for (const auto& line : lines)
   {
      if (line.find(tagToFind) == 0)
      {
         const string rawData = line.substr(tagToFind.size());
         if (rawData.front() == '"' && rawData.back() == '"')
            return rawData.substr(1,rawData.size()-2);
         return rawData;
      }
   }
   return "Unknown";
}
