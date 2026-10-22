#!/bin/sh
set -e

EMCC=emcc
EMCXX=em++

# Export EXR loader/saver function to JS.
# TODO: export more functions
# DEMANGLE_SUPPORT=1
#${EMCC} -std=c++11 --bind -O2 -I../../ binding.cc --memory-init-file 0 -s TOTAL_MEMORY=67108864 -s DEMANGLE_SUPPORT=1 -s EXPORTED_FUNCTIONS="['_ParseEXRHeaderFromMemory', '_LoadEXRFromMemory']" -o tinyexr.js
#${EMCC} --bind -Os -I../../ -I../../deps/miniz binding.cc ../../deps/miniz/miniz.c --memory-init-file 0 -s TOTAL_MEMORY=67108864 -o tinyexr.js

"${EMCC}" -Os "-I../../deps/miniz" "../../deps/miniz/miniz.c" -c -o miniz.o
"${EMCXX}" --bind -Os "-I../../" "-I../../deps/miniz" binding.cc miniz.o -s ALLOW_MEMORY_GROWTH=1 -o tinyexr.js --emit-tsd tinyexr.d.ts

