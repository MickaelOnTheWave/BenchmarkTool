#include "ZipArchive.h"

#include <cstring>
#include "miniz.h"

struct ZipArchive::Impl
{
   mz_zip_archive archive{};
   bool valid = false;
   std::vector<std::string> files;
};

ZipArchive::ZipArchive(const ByteBuffer& data)
   : impl(new Impl)
{
   impl->valid = mz_zip_reader_init_mem(
      &impl->archive,
      data.data(),
      data.size(),
      0
      );

   if (!impl->valid)
      return;

   const int fileCount = (int)mz_zip_reader_get_num_files(&impl->archive);

   impl->files.reserve(fileCount);

   for (int i = 0; i < fileCount; ++i)
   {
      mz_zip_archive_file_stat stat;
      std::memset(&stat, 0, sizeof(stat));

      if (!mz_zip_reader_file_stat(&impl->archive, i, &stat))
         continue;

      impl->files.emplace_back(stat.m_filename);
   }
}

ZipArchive::~ZipArchive()
{
   if (impl)
   {
      if (impl->valid)
         mz_zip_reader_end(&impl->archive);
   }
   delete impl;
}

bool ZipArchive::IsValid() const
{
   return impl && impl->valid;
}

bool ZipArchive::ContainsFile(const std::string& name) const
{
   if (!impl || !impl->valid)
      return false;

   for (const auto& f : impl->files)
   {
      if (f == name)
         return true;
   }

   return false;
}

std::vector<std::string> ZipArchive::ListFiles() const
{
   if (!impl || !impl->valid)
      return {};

   return impl->files;
}

ByteBuffer ZipArchive::ExtractFile(const std::string& name) const
{
   ByteBuffer result;

   if (!impl || !impl->valid)
      return result;

   size_t size = 0;

   void* data = mz_zip_reader_extract_file_to_heap(
      &impl->archive,
      name.c_str(),
      &size,
      0
      );

   if (!data || size == 0)
      return result;

   result.resize(size);
   std::memcpy(result.data(), data, size);

   mz_free(data);

   return result;
}
