#!/bin/bash

generator="$1"
dir="$(cd "$(dirname "$0")"; pwd)"

mkdir -p deps/leveldb
pushd deps/leveldb
if [ -z "$generator" ]; then
	cmake "$dir/ext/leveldb" -DLEVELDB_SNAPPY=OFF
else
	cmake "$dir/ext/leveldb" -DLEVELDB_SNAPPY=OFF -G "$generator"
fi
cmake --build . --target leveldb --config Debug
cmake --build . --target leveldb --config Release
popd

if [ -z "$generator" ]; then
	cmake "$dir"
else
	cmake "$dir" -G "$generator"
fi
