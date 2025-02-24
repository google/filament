#!/bin/bash

BUILDID=5528
BUILD_PATH=https://storage.googleapis.com/wasm-llvm/builds/git

wget -O - $BUILD_PATH/wasm-torture-s-$BUILDID.tbz2 \
  | tar xj

wget -O - $BUILD_PATH/wasm-torture-s2wasm-sexpr-wasm-$BUILDID.tbz2 \
  | tar xj

wget -O - $BUILD_PATH/wasm-binaries-$BUILDID.tbz2 \
  | tar xj

wget -O - \
  $BUILD_PATH/wasm-torture-/b/build/slave/linux/build/src/src/work/wasm-install/emscripten_config_vanilla-$BUILDID.tbz2 \
  | tar xj
