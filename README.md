# LevelDB with zlib support

A fork of LevelDB that compatible with Minecraft PE / Bedrock.

## Motivation

Mojang's [leveldb-mcpe](https://github.com/Mojang/leveldb-mcpe) lacks support of CMake so I created this repo with latest leveldb.

## Getting Started

### Prerequisities

* You may need [vcpkg](https://github.com/microsoft/vcpkg) to build it on Windows

* zlib

### Build

generate solution with cmake and vcpkg:

```
cmake .. -DCMAKE_TOOLCHAIN_FILE=<vcpkg root directory>\scripts\buildsystems\vcpkg.cmake
```

### Integrate with vcpkg

see [leveldb-mcpe-portfile](https://github.com/jasper-wan/leveldb-mcpe-portfile).
