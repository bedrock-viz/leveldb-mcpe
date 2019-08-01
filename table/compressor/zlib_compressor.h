#ifndef STORAGE_LEVELDB_TABLE_ZLIB_COMPRESSOR_H_
#define STORAGE_LEVELDB_TABLE_ZLIB_COMPRESSOR_H_

#include <string>

namespace leveldb {

bool Zlib_Compress(int compressionLevel, const char* input, size_t length,
                   std::string& output);

bool ZlibRaw_Compress(int compressionLevel, const char* input, size_t length,
                      std::string& output);

bool Zlib_Uncompress(const char* input, size_t length, std::string& output);

bool ZlibRaw_Uncompress(const char* input, size_t length, std::string& output);

}  // namespace leveldb
#endif  // STORAGE_LEVELDB_TABLE_ZLIB_COMPRESSOR_H_