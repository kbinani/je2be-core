mkdir build
mkdir build\deps
pushd build\deps

rem zlib
mkdir zlib
pushd zlib
cmake ..\..\..\ext\zlib
cmake --build .
popd

rem leveldb
mkdir leveldb
pushd leveldb
cmake ..\..\..\ext\leveldb -DZLIB_LIBRARY=..\zlib\Debug\zlib -DZLIB_INCLUDE_DIR=..\zlib
cmake --build . --target leveldb
popd

popd
