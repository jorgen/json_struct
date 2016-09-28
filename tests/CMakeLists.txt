add_executable(json-tokenizer-fail-test json-tokenizer-fail-test.cpp)
add_executable(json-tokenizer-partial-test json-tokenizer-partial-test.cpp)
add_executable(json-tokenizer-test json-tokenizer-test.cpp)
add_executable(json-struct-test json-struct-test.cpp)

include_directories(${JSON_TOOLS_INCLUDE_DIR})

add_test(NAME json-tokenizer-fail-test COMMAND json-tokenizer-fail-test)
add_test(NAME json-tokenizer-partial-test COMMAND json-tokenizer-partial-test)
add_test(NAME json-tokenizer-test COMMAND json-tokenizer-test)
add_test(NAME json-struct-test COMMAND json-struct-test)
