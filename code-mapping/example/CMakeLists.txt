cmake_minimum_required(VERSION 3.20)
project(sample_shop CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_subdirectory(core)
add_subdirectory(storage)
add_subdirectory(service)
add_subdirectory(app)
