cmake_minimum_required(VERSION 3.20)

project(LLOrderbook)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)

add_subdirectory(llob)
