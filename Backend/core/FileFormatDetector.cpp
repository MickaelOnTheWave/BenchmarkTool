#include "FileFormatDetector.h"

#include "ZipArchive.h"

ImportFormat FileFormatDetector::Detect(const std::string& filename,
                                        const ByteBuffer& content)
{
   if (Is3DMarkFile(filename, content))
      return ImportFormat::_3DMark;

   return ImportFormat::Unknown;
}

bool FileFormatDetector::Is3DMarkFile(const std::string& filename,
                                      const ByteBuffer& content)
{
   (void)filename;

   ZipArchive currentFile(content);
   if (currentFile.IsValid())
   {
      if (currentFile.ContainsFile("Arielle.xml") && currentFile.ContainsFile("Monitoring.csv") &&
         currentFile.ContainsFile("Result.xml") && currentFile.ContainsFile("SI.xml"))
      {
         return true;
      }
   }
   return false;
}
