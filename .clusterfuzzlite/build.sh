#!/bin/bash -eu
# Copy fuzzer executables to $OUT/
$CXX $CFLAGS $LIB_FUZZING_ENGINE \
  $SRC/json_struct/.clusterfuzzlite/reformat_fuzzer.cpp \
  -o $OUT/reformat_fuzzer \
  -I$SRC/json_struct/include
