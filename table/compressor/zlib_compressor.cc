#include "zlib_compressor.h"

#include <algorithm>
#include <cassert>

#include <zlib.h>

namespace {
bool Zlib_UncompressImpl(const char* input, size_t length, std::string& output,
                         bool raw) {
  const int CHUNK = 64 * 1024;

  int ret;
  size_t have;
  z_stream strm;
  unsigned char* out = new unsigned char[CHUNK];

  /* allocate inflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = (uint32_t)length;
  strm.next_in = (Bytef*)input;

  ret = inflateInit2(&strm, raw ? -15 : 15);

  if (ret != Z_OK) {
    return ret;
  }

  /* decompress until deflate stream ends or end of file */
  do {
    /* run inflate() on input until output buffer not full */
    do {
      strm.avail_out = CHUNK;
      strm.next_out = out;

      ret = ::inflate(&strm, Z_NO_FLUSH);

      if (ret == Z_NEED_DICT) {
        ret = Z_DATA_ERROR;
      }
      if (ret < 0) {
        (void)inflateEnd(&strm);
        delete[] out;
        return ret;
      }

      have = CHUNK - strm.avail_out;

      output.append((char*)out, have);

    } while (strm.avail_out == 0);

    /* done when inflate() says it's done */
  } while (ret != Z_STREAM_END);

  /* clean up and return */
  (void)inflateEnd(&strm);
  delete[] out;
  return ret == Z_STREAM_END;
}

bool Zlib_CompressImpl(int compressionLevel, const char* input, size_t length,
                       std::string& buffer, bool raw) {
  const size_t BUFSIZE = 128 * 1024;
  unsigned char* temp_buffer = new unsigned char[BUFSIZE];

  // reserve enough memory to not reallocate on the fly
  buffer.reserve(buffer.size() + compressBound(length));

  z_stream strm;
  strm.zalloc = 0;
  strm.zfree = 0;
  strm.next_in = (unsigned char*)(input);
  strm.avail_in = (uint32_t)length;
  strm.next_out = temp_buffer;
  strm.avail_out = BUFSIZE;

  auto res = deflateInit2(&strm, compressionLevel, Z_DEFLATED, raw ? -15 : 15,
                          8, Z_DEFAULT_STRATEGY);
  assert(res == Z_OK);

  int deflate_res = Z_OK;
  while (strm.avail_in != 0) {
    int res = deflate(&strm, Z_NO_FLUSH);
    assert(res == Z_OK);
    if (strm.avail_out == 0) {
      buffer.append(temp_buffer, temp_buffer + BUFSIZE);
      strm.next_out = temp_buffer;
      strm.avail_out = BUFSIZE;
    }
  }

  while (deflate_res == Z_OK) {
    if (strm.avail_out == 0) {
      buffer.append(temp_buffer, temp_buffer + BUFSIZE);
      strm.next_out = temp_buffer;
      strm.avail_out = BUFSIZE;
    }
    deflate_res = deflate(&strm, Z_FINISH);
  }

  assert(deflate_res == Z_STREAM_END);
  buffer.append(temp_buffer, temp_buffer + BUFSIZE - strm.avail_out);
  deflateEnd(&strm);
  delete[] temp_buffer;
  return true;
}
}  // namespace

namespace leveldb {
bool Zlib_Compress(int compressionLevel, const char* input, size_t length,
                   std::string& output) {
  return Zlib_CompressImpl(compressionLevel, input, length, output, false);
}

bool ZlibRaw_Compress(int compressionLevel, const char* input, size_t length,
                      std::string& output) {
  return Zlib_CompressImpl(compressionLevel, input, length, output, true);
}

bool Zlib_Uncompress(const char* input, size_t length, std::string& output) {
  return Zlib_UncompressImpl(input, length, output, false);
}

bool ZlibRaw_Uncompress(const char* input, size_t length, std::string& output) {
  return Zlib_UncompressImpl(input, length, output, true);
}
}  // namespace leveldb