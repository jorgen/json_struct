file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(jsonmod ${SRC_FILES} ${JSONTOOLS_FILES})
target_include_directories(jsonmod PRIVATE ${JSON_TOOLS_INCLUDE_DIR})
