cmake_minimum_required(VERSION 3.20)

project(LLOrderbook)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS container)
find_package(Boost REQUIRED COMPONENTS unordered)

add_subdirectory(llob)
add_subdirectory(tests)
add_subdirectory(benchmarks)
