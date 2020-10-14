cmake_minimum_required(VERSION 3.0)
project(je2be)

include(CheckCCompilerFlag)

include_directories(. ext/libminecraft-file/include ext/leveldb-mcpe/include)
link_directories(ext/leveldb-mcpe/out-static)
add_definitions(-std=c++17 -DDLLX=)
add_executable(je2be src/main.cpp ext/libminecraft-file/include/minecraft-file.hpp)
list(APPEND je2be_link_libraries "z;leveldb")

set(CMAKE_REQUIRED_FLAGS "-lstdc++fs")
check_c_compiler_flag("" TEST_RESULT_STDCXXFS)
if (TEST_RESULT_STDCXXFS AND "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  list(APPEND je2be_link_libraries stdc++fs)
endif()

target_link_libraries(je2be ${je2be_link_libraries})
