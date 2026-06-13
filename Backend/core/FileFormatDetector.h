#ifndef FILEFORMATDETECTOR_H
#define FILEFORMATDETECTOR_H

#include <stdint.h>
#include <string>
#include <vector>

using ByteBuffer = std::vector<uint8_t>;

enum class ImportFormat
{
   Unknown,
   _3DMark
};

class FileFormatDetector
{
public:
   static ImportFormat Detect(const std::string& filename,
                              const ByteBuffer& content);

private:
   static bool Is3DMarkFile(const std::string& filename,
                            const ByteBuffer& content);
};

#endif // FILEFORMATDETECTOR_H
