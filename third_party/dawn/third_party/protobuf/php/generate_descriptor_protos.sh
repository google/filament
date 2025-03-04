#!/usr/bin/env bash

# Run this script to regenerate descriptor protos after the protocol compiler
# changes.

set -e

if test ! -e src/google/protobuf/stubs/common.h; then
  cat >&2 << __EOF__
Could not find source code.  Make sure you are running this script from the
root of the distribution tree.
__EOF__
  exit 1
fi

pushd src
./protoc --php_out=internal:../php/src google/protobuf/descriptor.proto
./protoc --php_out=internal_generate_c_wkt:../php/src \
  google/protobuf/any.proto \
  google/protobuf/api.proto \
  google/protobuf/duration.proto \
  google/protobuf/empty.proto \
  google/protobuf/field_mask.proto \
  google/protobuf/source_context.proto \
  google/protobuf/struct.proto \
  google/protobuf/type.proto \
  google/protobuf/timestamp.proto \
  google/protobuf/wrappers.proto
popd
