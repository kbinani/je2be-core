#!/bin/bash

generator="$1"
dir="$(cd "$(dirname "$0")"; pwd)"

mkdir -p deps/leveldb
pushd deps/leveldb
if [ -z "$generator" ]; then
	for type in Debug Release; do
		mkdir $type
		pushd $type
		cmake "$dir/ext/leveldb" -DLEVELDB_SNAPPY=OFF -DLEVELDB_ZSTD=OFF -DCMAKE_BUILD_TYPE=$type
		cmake --build . --target leveldb --config $type --parallel $(nproc)
		popd
	done
else
	cmake "$dir/ext/leveldb" -DLEVELDB_SNAPPY=OFF -DLEVELDB_ZSTD=OFF -G "$generator"
	cmake --build . --target leveldb --config Debug --parallel $(nproc)
	cmake --build . --target leveldb --config Release --parallel $(nproc)
fi
popd

mkdir -p deps/xxHash
pushd deps/xxHash

if [ -z "$generator" ]; then
	for type in Debug Release; do
		mkdir $type
		pushd $type
		cmake "$dir/ext/xxHash/cmake_unofficial" -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=$type
		cmake --build . --target xxhash --config $type --parallel $(nproc)
		popd
	done
else
	cmake "$dir/ext/xxHash/cmake_unofficial" -DBUILD_SHARED_LIBS=OFF -G "$generator"
	cmake --build . --target xxhash --config Debug --parallel $(nproc)
	cmake --build . --target xxhash --config Release --parallel $(nproc)
fi
popd
