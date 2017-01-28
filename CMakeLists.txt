cmake_minimum_required (VERSION 3.0)
project (json_tools)

set (CMAKE_CXX_STANDARD 11)
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

set(JSON_TOOLS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package (LLVM HINTS $ENV{LLVM_MODULE})
find_package (Clang HINTS $ENV{LLVM_MODULE})

add_subdirectory(jsonmod)
if (LLVM_FOUND AND Clang_FOUND)
	add_subdirectory(extractor)
endif()
enable_testing()
add_subdirectory(tests)
