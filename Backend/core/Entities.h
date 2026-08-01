#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>

struct Machine
{
   int id;
   std::string name;
   std::string cpu;
   std::string gpu;
   int ramGb;
   std::string motherboard;
};

struct HardwareConfig
{
   int id;
   std::string name;
   int machineId;
   double cpuFreqGhz;
   int gpuFreqMhz;
   int ramFreqMhz;
   std::string settings;
};

struct SoftwareEnvironment
{
   int id;
   std::string name;
   std::string os;
   std::string osVersion;
   std::string driverFamily;
};

struct SoftwareConfig
{
   int id;
   std::string name;
   int softwareEnvironmentId;
   std::string driverVersion;
   std::string mode;
   std::string settings;
};

#endif // ENTITIES_H
