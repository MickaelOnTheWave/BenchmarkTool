#ifndef ZIP_ARCHIVE_H
#define ZIP_ARCHIVE_H

#include <string>
#include <vector>
#include <cstdint>

using ByteBuffer = std::vector<std::uint8_t>;

class ZipArchive
{
public:
   explicit ZipArchive(const ByteBuffer& data);
   virtual ~ZipArchive();

   bool IsValid() const;

   bool ContainsFile(const std::string& name) const;

   std::vector<std::string> ListFiles() const;

   ByteBuffer ExtractFile(const std::string& name) const;

private:
   // Using PIMPL to not leak miniz dependency outside of the implementation
   struct Impl;
   Impl* impl;
};

#endif
