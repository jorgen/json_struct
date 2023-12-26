#!/bin/bash -eu
# Copy fuzzer executables to $OUT/
$CXX $CXXFLAGS -fsanitize=fuzzer $SRC/json_struct/.clusterfuzzlite/reformat_fuzzer.cpp -o $OUT/reformat_fuzzer -I$SRC/json_struct/include
