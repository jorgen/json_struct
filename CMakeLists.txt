### Project
cmake_minimum_required(VERSION 3.16)
project(json_struct VERSION "1.0.0" LANGUAGES CXX)

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
function(set_compiler_flags_for_target target)
  set_property(TARGET ${target} PROPERTY CXX_STANDARD 11)
  if (MSVC)
    if(MSVC_VERSION GREATER_EQUAL 1920) #VS 2019 and above
      target_compile_options(${target} PRIVATE /W4 /WX)
    endif()
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()
endfunction()

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
  COMMAND clang-format -i include/json_struct/json_struct.h
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  SOURCES include/json_struct/json_struct.h)

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

    file(GLOB HEADERS_TO_INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/include/json_struct/*)
    install(FILES ${HEADERS_TO_INSTALL}
            DESTINATION include/${PROJECT_NAME})

    export(EXPORT ${PROJECT_NAME}Config
            NAMESPACE ${PROJECT_NAME}::)

    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.xml
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})
endif()
