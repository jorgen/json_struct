cmake_minimum_required (VERSION 2.8)
project (json_tools)

set (CMAKE_CXX_STANDARD 11)

set(JSON_TOOLS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(jsonmod)
add_subdirectory(extractor)
enable_testing()
add_subdirectory(tests)
