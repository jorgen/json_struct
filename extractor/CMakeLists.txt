
if (Clang_NOT_FOUND)
    return()
endif()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fno-rtti")
endif()
add_definitions(${LLVM_DEFINITIONS})
add_executable(extractor main.cpp
                         astvisitor.h
                         astvisitor.cpp
                         astmatcher.h
                         astmatcher.cpp
                         ${JSON_TOOLS_INCLUDE_DIR}/extractor/extractor_api.h)
target_include_directories(extractor PRIVATE
				     ${LLVM_INCLUDE_DIRS}
				     ${JSON_TOOLS_INCLUDE_DIR})
target_link_libraries(extractor
                      clangTooling
                      clangBasic
                      clangASTMatchers
                      LLVMX86AsmParser
                      LLVMTarget
                      LLVMX86AsmParser
                      LLVMX86AsmPrinter
                      LLVMX86CodeGen
                      LLVMX86Desc
                      LLVMX86Disassembler
                      LLVMX86Info
                      LLVMX86Utils
                      )
