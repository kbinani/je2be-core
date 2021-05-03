rem This is a setup script for out-of-source build.
rem Call this script in your build directory.

rem Example:
rem cd C:\your\build\directory
rem C:\location\of\this\script\setup.bat
rem start je2be.sln

@echo off
setlocal enabledelayedexpansion

mkdir deps
pushd deps

set dir=%~dp0

rem zlib
mkdir zlib
pushd zlib
cmake %dir%\ext\zlib
cmake --build . --config Debug
cmake --build . --config Release
popd

rem leveldb
mkdir leveldb
pushd leveldb
cmake %dir%\ext\leveldb -DZLIB_LIBRARY=..\zlib\Release\zlib -DZLIB_INCLUDE_DIR=..\zlib
cmake --build . --target leveldb --config Debug
cmake --build . --target leveldb --config Release
popd

rem xxHash
mkdir xxHash
pushd xxHash
cmake %dir%\ext\xxHash\cmake_unofficial -DBUILD_SHARED_LIBS=OFF
cmake --build . --target xxhash --config Debug
cmake --build . --target xxhash --config Release
popd

popd
