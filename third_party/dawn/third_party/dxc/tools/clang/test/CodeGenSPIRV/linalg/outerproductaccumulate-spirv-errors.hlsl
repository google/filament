// RUN: %dxc -I %hlsl_headers -T lib_6_9 -enable-16bit-types -spirv %s -verify

// Tests that the header file cannot be included for spirv compilations
// This is a copy of \tools\clang\test\CodeGenDXIL\hlsl\linalg\outerproductaccumulate.hlsl
// except that spirv is targeted

// expected-error@dx/linalg.h:4{{Cooperative vectors not (yet) supported for SPIRV}}
#include <dx/linalg.h>

RWByteAddressBuffer RWBuf;

export void Test4(vector<half, 128> Input1, vector<half, 64> Input2) {
  using namespace dx::linalg;

  RWMatrixRef<DATA_TYPE_FLOAT16, 128, 64, MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL>
      matrix = {RWBuf, 0, 0};

  OuterProductAccumulate(Input1, Input2, matrix);  
}
