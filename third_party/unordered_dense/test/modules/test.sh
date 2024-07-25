#!/bin/bash
set -xe

rm -f *.o *.pcm a.out

clang++ -std=c++20 -I ../../include --precompile -x c++-module ../../src/ankerl.unordered_dense.cpp
clang++ -std=c++20 -c ankerl.unordered_dense.pcm
clang++ -std=c++20 -fprebuilt-module-path=. ankerl.unordered_dense.o module_test.cpp -o a.out

./a.out

rm -f *.o *.pcm a.out
