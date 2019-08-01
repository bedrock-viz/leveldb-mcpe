#include <iostream>
#include <sstream>

#include <leveldb/c.h>
#include <leveldb/cache.h>
#include <leveldb/db.h>
#include <leveldb/decompress_allocator.h>
#include <leveldb/env.h>
#include <leveldb/filter_policy.h>
#include <leveldb/options.h>

namespace bedrock {
enum class Tag : char {
  Data2D = 45,
  Data2DLegacy = 46,
  SubChunkPrefix = 47,
  LegacyTerrain = 48,
  BlockEntity = 49,
  Entity = 50,
  PendingTicks = 51,
  BlockExtraData = 52,
  BiomeState = 53,
  FinalizedState = 54,
  Version = 118
};
}

union Coord {
  int num;
  char bin[4];
};

std::string TagToString(const bedrock::Tag& tag) {
  using namespace bedrock;
  switch (tag) {
    case Tag::Data2D:
      return "Data2D";
    case Tag::Data2DLegacy:
      return "Data2DLegacy";
    case Tag::SubChunkPrefix:
      return "SubChunkPrefix";
    case Tag::LegacyTerrain:
      return "LegacyTerrain";
    case Tag::BlockEntity:
      return "BlockEntity";
    case Tag::Entity:
      return "Entity";
    case Tag::PendingTicks:
      return "PendingTicks";
    case Tag::BlockExtraData:
      return "BlockExtraData";
    case Tag::BiomeState:
      return "BiomeState";
    case Tag::FinalizedState:
      return "FinalizedState";
    case Tag::Version:
      return "Version";
  }
  return "Unknown";
}

std::string PrintKeyInfo(const std::string& key) {
  auto mainWorld = !(key.length() > 4 + 4 + 4 + 1);

  Coord x;
  x.bin[0] = key[0];
  x.bin[1] = key[1];
  x.bin[2] = key[2];
  x.bin[3] = key[3];
  Coord z;
  z.bin[0] = key[4];
  z.bin[1] = key[5];
  z.bin[2] = key[6];
  z.bin[3] = key[7];

  const int tagIndex = (mainWorld ? 8 : 24);
  if (key.length() - 1 < tagIndex) {
    std::stringstream ss;
    ss << "Unknown key: " << key;
    return ss.str();
  }
  const auto tag = bedrock::Tag(key[tagIndex]);
  const int subtrunkIdIndex = tagIndex + 1;
  char buffer[100];
  if (tag == bedrock::Tag::SubChunkPrefix) {
    snprintf(buffer, 100, "X: %d, Z: %d, Tag: %s, SubTrunkID: %d", x.num, z.num,
             TagToString(tag).c_str(), key[subtrunkIdIndex]);
  } else {
    snprintf(buffer, 100, "X: %d, Z: %d, Tag: %s", x.num, z.num,
             TagToString(tag).c_str());
  }

  return std::string(buffer);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Invalid database\n");
    return 1;
  }

  auto path = std::string(argv[1]);

  class NullLogger : public leveldb::Logger {
   public:
    void Logv(const char*, va_list) override {}
  };

  leveldb::Options options;

  // create a bloom filter to quickly tell if a key is in the database or not
  options.filter_policy = leveldb::NewBloomFilterPolicy(10);

  // create a 40 mb cache (we use this on ~1gb devices)
  options.block_cache = leveldb::NewLRUCache(40 * 1024 * 1024);

  // create a 4mb write buffer, to improve compression and touch the disk less
  options.write_buffer_size = 4 * 1024 * 1024;

  // disable internal logging. The default logger will still print out things to
  // a file
  options.info_log = new NullLogger();

  options.zlib_compression_level = -1;
  options.compression = leveldb::CompressionType::kZlibRawCompression;

  // create a reusable memory space for decompression so it allocates less
  leveldb::ReadOptions readOptions;
  // readOptions.decompress_allocator = new leveldb::DecompressAllocator();

  //... init leveldb with Options and read with ReadOptions
  leveldb::DB* db = nullptr;
  auto s = leveldb::DB::Open(options, path, &db);
  if (!s.ok()) {
    fprintf(stderr, "open error: %s\n", s.ToString().c_str());
    exit(1);
  }

  auto iter = db->NewIterator(readOptions);
  int count = 0;
  for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
    const auto key = iter->key();
    if (key == "AutonomousEntities") {
      printf("AutonomousEntities\n");
    } else if (key == "Nether") {
      printf("Nether\n");
    } else if (key == "TheEnd") {
      printf("TheEnd\n");
    } else {
      std::cout << PrintKeyInfo(key.ToString()) << '\n';
    }
  }

  return 0;
}