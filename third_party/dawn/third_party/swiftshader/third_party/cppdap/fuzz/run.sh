#!/bin/bash

set -e # Fail on any error.

FUZZ_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"
cd ${FUZZ_DIR}

# Ensure we're testing with latest build
[ ! -d "build" ] && mkdir "build"
cd "build"
cmake ../.. -GNinja -DCPPDAP_BUILD_FUZZER=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
ninja

cd ${FUZZ_DIR}
[ ! -d "corpus" ] && mkdir "corpus"
[ ! -d "logs" ] && mkdir "logs"
cd "logs"
rm crash-* fuzz-* || true
${FUZZ_DIR}/build/cppdap-fuzzer ${FUZZ_DIR}/corpus ${FUZZ_DIR}/seed -dict=${FUZZ_DIR}/dictionary.txt -jobs=128
