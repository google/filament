#!/bin/bash

. /bin/using.sh

set -x # Display commands being run.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"

using clang-13.0.1

# Set up env vars
export CLANG_FORMAT=`which clang-format`

# Run presubmit tests
cd git/SwiftShader
./tests/presubmit.sh
