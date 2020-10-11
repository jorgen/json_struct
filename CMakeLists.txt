cmake_minimum_required (VERSION 3.0)
project (json_struct)

set (CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-old-style-cast -Wno-shadow-field-in-constructor -Wno-switch-enum -Wno-padded -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic")
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	add_compile_options(/permissive-)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
else()
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-old-style-cast -Wno-shadow-field-in-constructor -Wno-switch-enum -Wno-padded -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic")
endif()
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

set(JSON_STRUCT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(examples)
enable_testing()
add_subdirectory(tests)

set(BUILD_PERFORMANCE ON)
if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
  set(BUILD_PERFORMANCE OFF)
endif()
if(BUILD_PERFORMANCE)
  add_subdirectory(performance)
endif()

add_custom_target(clangformat
  COMMAND clang-format -i include/json_struct.h
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  SOURCES include/json_struct.h)

