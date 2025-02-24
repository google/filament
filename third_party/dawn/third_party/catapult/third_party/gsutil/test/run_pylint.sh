#!/bin/bash

# Assumes this script is in a directory one level below the repo's root dir.
TEST_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
GSUTIL_DIR="$TEST_DIR/.."

find "$GSUTIL_DIR" \
    -path "$GSUTIL_DIR/gslib/third_party" -prune -o \
    -path "$GSUTIL_DIR/gslib/vendored" -prune -o \
    -path "$GSUTIL_DIR/third_party" -prune -o \
    -name "*.py" -print \
  | xargs python3 -m pylint --rcfile="$TEST_DIR/.pylintrc_limited"
