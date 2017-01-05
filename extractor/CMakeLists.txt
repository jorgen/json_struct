find_package (LLVM REQUIRED)
find_package (Clang REQUIRED)

if (Clang_NOT_FOUND)
    return()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fno-rtti")
add_definitions(${LLVM_DEFINITIONS})
add_executable(extractor main.cpp
                         astvisitor.h
                         astvisitor.cpp
                         astmatcher.h
                         astmatcher.cpp)
target_include_directories(extractor PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries(extractor
                      clangTooling
                      clangBasic
                      clangASTMatchers)
