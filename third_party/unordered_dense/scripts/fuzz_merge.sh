#!/bin/env bash
set -e

# Start from a build directory, usually clang_cpp17_release
# usage: fuzz_merge.sh <testname>
FUZZ_TARGET=$1
SCRIPT_DIR=`dirname "$0"`
CORPUS_SMALL=${SCRIPT_DIR}/../data/fuzz/${FUZZ_TARGET}
CORPUS_BIG=CORPUS_BIG/${FUZZ_TARGET}
ninja
./test/fuzz_${FUZZ_TARGET} -merge=1 ${CORPUS_SMALL} ${CORPUS_BIG}
