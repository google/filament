// RUN: %dxc -I %hlsl_headers -T lib_6_9 -enable-16bit-types %s -verify

#include <dx/linalg.h>

RWByteAddressBuffer RWBuf;

// test for inputs of different size
export void Test4(vector<half, 128> Input1, vector<half, 64> Input2) {
  using namespace dx::linalg;

  RWMatrixRef<DATA_TYPE_FLOAT16, 128, 64, MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL, true>
      matrix = {RWBuf, 0, 0};

  // expected-error@+3{{no matching function for call to 'OuterProductAccumulate'}}
  // expected-note@dx/linalg.h:177{{candidate template ignored: could not match 0 against 1}}

  OuterProductAccumulate(Input1, Input2, matrix);  
}

// now test for an error when element types differ
export void Test5(vector<int, 128> Input1, vector<uint, 128> Input2) {
  using namespace dx::linalg;

  RWMatrixRef<DATA_TYPE_FLOAT16, 128, 128, MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL, true>
      matrix = {RWBuf, 0, 0};

  // expected-error@+3{{no matching function for call to 'OuterProductAccumulate'}}
  // expected-note@dx/linalg.h:177{{candidate template ignored: could not match 0 against 1}}

  OuterProductAccumulate(Input1, Input2, matrix);  
}

// now test for an error when matrix transpose parameter is true
export void Test4(vector<half, 64> Input1, vector<half, 64> Input2) {
  using namespace dx::linalg;

  RWMatrixRef<DATA_TYPE_FLOAT16, 64, 64, MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL, true>
      matrix = {RWBuf, 0, 0};

  // expected-error@+3{{no matching function for call to 'OuterProductAccumulate'}}
  // expected-note@dx/linalg.h:177{{candidate template ignored: deduced conflicting types for parameter 'ElTy' ('int' vs. 'unsigned int')}}

  OuterProductAccumulate(Input1, Input2, matrix);  
}
