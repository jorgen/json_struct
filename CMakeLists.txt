cmake_minimum_required (VERSION 3.0)
project (json_tools)

set (CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-old-style-cast -Wno-shadow-field-in-constructor -Wno-switch-enum -Wno-padded -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic")
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

set(JSON_TOOLS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(ENABLE_EXTRACTOR "Enable building ectractor \"Requires LLVM and Clang\"" NO)
if (ENABLE_EXTRACTOR)
    find_package (LLVM HINTS $ENV{LLVM_MODULE} REQUIRED)
    find_package (Clang HINTS $ENV{LLVM_MODULE} REQUIRED)
endif()

add_subdirectory(jsonmod)
add_subdirectory(examples)
if (ENABLE_EXTRACTOR)
	add_subdirectory(extractor)
endif()
enable_testing()
add_subdirectory(tests)
