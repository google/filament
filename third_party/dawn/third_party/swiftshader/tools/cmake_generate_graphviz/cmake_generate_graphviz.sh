#!/bin/bash

set -e # Fail on any error.
set -x # Display commands being run.

pushd `dirname $0`

if ! [ -x "$(command -v cmake)" ]; then
  echo 'cmake is not found. Please install it (e.g. sudo apt install cmake)' >&2
  exit 1
fi

if ! [ -x "$(command -v dot)" ]; then
  echo 'graphviz (dot) is not found. Please install it (e.g. sudo apt install graphviz)' >&2
  exit 1
fi

cmake_binary_dir=$1

if [[ -z "${cmake_binary_dir}" ]]; then
  cmake_binary_dir="../../build"
fi

cp ./CMakeGraphVizOptions.cmake ${cmake_binary_dir}/

pushd ${cmake_binary_dir}

cmake --graphviz=SwiftShader.dot ..
dot -Tpng -o SwiftShader.png SwiftShader.dot

if [ "$(uname)" == "Darwin" ]; then
  open SwiftShader.png
else
  xdg-open SwiftShader.png &>/dev/null &
fi

popd
popd
