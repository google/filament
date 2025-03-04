#!/bin/bash

function generate_proto() {
  PROTOC1=$1
  PROTOC2=$2

  rm -rf generated
  mkdir generated

  $PROTOC1 --php_out=generated proto/test_include.proto
  $PROTOC2 --php_out=generated                 \
    -I../../src -I.                            \
    proto/empty/echo.proto                     \
    proto/test.proto                           \
    proto/test_no_namespace.proto              \
    proto/test_prefix.proto                    \
    proto/test_php_namespace.proto             \
    proto/test_empty_php_namespace.proto       \
    proto/test_reserved_enum_lower.proto       \
    proto/test_reserved_enum_upper.proto       \
    proto/test_reserved_enum_value_lower.proto \
    proto/test_reserved_enum_value_upper.proto \
    proto/test_reserved_message_lower.proto    \
    proto/test_reserved_message_upper.proto    \
    proto/test_service.proto                   \
    proto/test_service_namespace.proto         \
    proto/test_wrapper_type_setters.proto      \
    proto/test_descriptors.proto

  pushd ../../src
  $PROTOC2 --php_out=../php/tests/generated -I../php/tests -I. ../php/tests/proto/test_import_descriptor_proto.proto
  popd
}

# Remove tests to expect error. These were added to API tests by mistake.
function remove_error_test() {
  local TEMPFILE=`tempfile`
  cat $1 | \
  awk -v file=`basename $1` -v dir=`basename $(dirname $1)` '
    BEGIN {
      show = 1
    }
    /@expectedException PHPUnit_Framework_Error/ { show = 0; next; }
    / *\*\//                                     { print; next; }
    / *}/ {
      if (!show) {
        show = 1;
        next;
      }
    }
    show { print }
  ' > $TEMPFILE
  cp $TEMPFILE $1
}

set -ex

# Change to the script's directory.
cd $(dirname $0)

OLD_VERSION=$1
OLD_VERSION_PROTOC=https://repo1.maven.org/maven2/com/google/protobuf/protoc/$OLD_VERSION/protoc-$OLD_VERSION-linux-x86_64.exe

# Extract the latest protobuf version number.
VERSION_NUMBER=`grep "PHP_PROTOBUF_VERSION" ../ext/google/protobuf/protobuf.h | sed "s|#define PHP_PROTOBUF_VERSION \"\(.*\)\"|\1|"`

echo "Running compatibility tests between current $VERSION_NUMBER and released $OLD_VERSION"

# Check protoc
[ -f ../../src/protoc ] || {
  echo "[ERROR]: Please build protoc first."
  exit 1
}

# Download old test.
rm -rf protobuf
git clone https://github.com/protocolbuffers/protobuf.git
pushd protobuf
git checkout v$OLD_VERSION
popd

# Build and copy the new runtime
pushd ../ext/google/protobuf
make clean || true
phpize && ./configure && make
popd

rm -rf protobuf/php/ext
rm -rf protobuf/php/src
cp -r ../ext protobuf/php/ext/
cp -r ../src protobuf/php/src/

# Download old version protoc compiler (for linux)
wget $OLD_VERSION_PROTOC -O old_protoc
chmod +x old_protoc

NEW_PROTOC=`pwd`/../../src/protoc
OLD_PROTOC=`pwd`/old_protoc
cd protobuf/php
composer install

# Remove implementation detail tests.
# TODO(teboring): Temporarily disable encode_decode_test.php. In 3.13.0-rc1,
# repeated primitive field encoding is changed to packed, which is a bug fix.
# However, this fails the compatibility test which hard coded old encoding.
# Will re-enable the test after making a release. After the version bump, the
# compatibility test will use the updated test code.
tests=( array_test.php generated_class_test.php map_field_test.php well_known_test.php )
sed -i.bak '/php_implementation_test.php/d' phpunit.xml
sed -i.bak '/generated_phpdoc_test.php/d' phpunit.xml
sed -i.bak '/encode_decode_test.php/d' phpunit.xml
sed -i.bak 's/generated_phpdoc_test.php//g' tests/test.sh
sed -i.bak 's/generated_service_test.php//g' tests/test.sh
sed -i.bak 's/encode_decode_test.php//g' tests/test.sh
sed -i.bak '/memory_leak_test.php/d' tests/test.sh
sed -i.bak '/^    public function testTimestamp()$/,/^    }$/d' tests/well_known_test.php
sed -i.bak 's/PHPUnit_Framework_TestCase/\\PHPUnit\\Framework\\TestCase/g' tests/array_test.php
sed -i.bak 's/PHPUnit_Framework_TestCase/\\PHPUnit\\Framework\\TestCase/g' tests/map_field_test.php
sed -i.bak 's/PHPUnit_Framework_TestCase/\\PHPUnit\\Framework\\TestCase/g' tests/test_base.php
for t in "${tests[@]}"
do
  remove_error_test tests/$t
done

cd tests

# Test A.1:
#   proto set 1: use old version
#   proto set 2 which may import protos in set 1: use old version
generate_proto $OLD_PROTOC $OLD_PROTOC
./test.sh
pushd ..
./vendor/bin/phpunit
popd

# Test A.2:
#   proto set 1: use new version
#   proto set 2 which may import protos in set 1: use old version
generate_proto $NEW_PROTOC $OLD_PROTOC
./test.sh
pushd ..
./vendor/bin/phpunit
popd

# Test A.3:
#   proto set 1: use old version
#   proto set 2 which may import protos in set 1: use new version
generate_proto $OLD_PROTOC $NEW_PROTOC
./test.sh
pushd ..
./vendor/bin/phpunit
popd
