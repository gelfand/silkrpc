#!/bin/bash

PROJECT_DIR=$PWD
rm -fr build_clang_coverage
mkdir build_clang_coverage
cd build_clang_coverage
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$PROJECT_DIR/cmake/clang-libcxx20-fpic.cmake -DSILKRPC_CLANG_COVERAGE=ON ..
cmake --build . --parallel
cmd/unit_test
mv default.profraw unit_test.profraw
llvm-profdata merge *.profraw -o profdata
llvm-cov export -instr-profile profdata -format=lcov cmd/unit_test > silkrpc.lcov
