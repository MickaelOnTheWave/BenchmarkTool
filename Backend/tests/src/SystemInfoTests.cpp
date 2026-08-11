#include <catch2/catch_test_macros.hpp>

#include <memory>

#ifdef __linux__
   #include "LinuxSystemInfo.h"
#endif

namespace
{
   std::unique_ptr<ISystemInfo> CreateSystemInfo()
   {
#ifdef __linux__
      return std::make_unique<LinuxSystemInfo>();
#endif
   }

   void CheckProcessingUnitInfo(const ProcessingUnitInfo& info)
   {
      REQUIRE(info.name != "");
      REQUIRE(info.maxFrequencyMhz >= info.minFrequencyMhz);
      REQUIRE(info.currentFrequencyMhz >= info.minFrequencyMhz);
      REQUIRE(info.currentFrequencyMhz <= info.maxFrequencyMhz);
   }
}

TEST_CASE("SystemInfo - CPU information")
{
   auto systemInfo = CreateSystemInfo();
   auto cpuInfo = systemInfo->GetCpu();

   REQUIRE(cpuInfo.minFrequencyMhz > 0);
   CheckProcessingUnitInfo(cpuInfo);
   REQUIRE(cpuInfo.coreCount > 0);
}

TEST_CASE("SystemInfo - GPU information")
{
   auto systemInfo = CreateSystemInfo();
   auto gpuInfo = systemInfo->GetGpu();

   CheckProcessingUnitInfo(gpuInfo);
}

TEST_CASE("SystemInfo - RAM information")
{
   auto systemInfo = CreateSystemInfo();
   auto ramInfo = systemInfo->GetRam();

   REQUIRE(ramInfo.quantityMb > 0);

   // Current detection requires root priviledges. We can't count on it.
   //REQUIRE(ramInfo.frequencyMhz > 0);
}
