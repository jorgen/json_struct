
### Project
cmake_minimum_required(VERSION 3.14)
project(json_struct VERSION "0.0.1" LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED FALSE)
set(CMAKE_CXX_STANDARD 20)

### Misc
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(IS_TOPLEVEL_PROJECT TRUE)
else()
    set(IS_TOPLEVEL_PROJECT FALSE)
endif()
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

### Options
option(JSON_STRUCT_OPT_BUILD_BENCHMARKS "Build json_struct benchmarks" ${IS_TOPLEVEL_PROJECT})
option(JSON_STRUCT_OPT_BUILD_EXAMPLES "Build json_struct examples" ${IS_TOPLEVEL_PROJECT})
option(JSON_STRUCT_OPT_BUILD_TESTS "Build and perform json_struct tests" ${IS_TOPLEVEL_PROJECT})
option(JSON_STRUCT_OPT_INSTALL "Generate and install json_struct target" ${IS_TOPLEVEL_PROJECT})

### Compiler
set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-old-style-cast -Wno-shadow-field-in-constructor -Wno-switch-enum -Wno-padded -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic")
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  add_compile_options(/permissive-)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
else()
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-old-style-cast -Wno-shadow-field-in-constructor -Wno-switch-enum -Wno-padded -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic")
endif()

### Include
set(JSON_STRUCT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

### Examples
if(JSON_STRUCT_OPT_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

### Tests
if(JSON_STRUCT_OPT_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

### Benchmarks
if(JSON_STRUCT_OPT_BUILD_BENCHMARKS)
  if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
    set(JSON_STRUCT_OPT_BUILD_BENCHMARKS OFF)
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.2)
    set(JSON_STRUCT_OPT_BUILD_BENCHMARKS OFF)
  endif()
endif()
if(JSON_STRUCT_OPT_BUILD_BENCHMARKS)
  add_subdirectory(performance)
endif()

### clangformat
add_custom_target(clangformat
  COMMAND clang-format -i include/json_struct.h
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  SOURCES include/json_struct.h)

### INSTALL
include(CMakePackageConfigHelpers)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

write_basic_package_version_file(${PROJECT_NAME}ConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
        ARCH_INDEPENDENT)

if(JSON_STRUCT_OPT_INSTALL)
    include(GNUInstallDirs)

    install(TARGETS ${PROJECT_NAME}
            EXPORT ${PROJECT_NAME}Config)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
            DESTINATION lib/cmake/${PROJECT_NAME})

    install(EXPORT ${PROJECT_NAME}Config
            NAMESPACE ${PROJECT_NAME}::
            DESTINATION lib/cmake/${PROJECT_NAME})

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
            DESTINATION .)

    export(EXPORT ${PROJECT_NAME}Config
            NAMESPACE ${PROJECT_NAME}::)

    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.xml
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})
endif()
