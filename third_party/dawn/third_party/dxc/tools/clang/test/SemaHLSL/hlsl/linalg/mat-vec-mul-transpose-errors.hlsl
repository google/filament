// XFAIL: *
// RUN: %dxc -I %hlsl_headers -T lib_6_9 -enable-16bit-types %s -verify

#include <dx/linalg.h>

ByteAddressBuffer Buf;

export float4 Test1(vector<float, 4> Input) {
  using namespace dx::linalg;

  MatrixRef<DATA_TYPE_FLOAT16, 4, 4, MATRIX_LAYOUT_ROW_MAJOR, true> Matrix = {
      Buf, 0, 0};

  // PREVIEW CHECK TODO:
  // expected-error@+1{{something about transposing not supported for rowmajor / colmajor layouts}}
  return Mul<float>(    
      Matrix, MakeInterpretedVector<DATA_TYPE_FLOAT16>(Input));
}

export vector<float, 8> Test2(vector<uint8_t4_packed, 6> Input) {
  using namespace dx::linalg;

  MatrixRef<DATA_TYPE_UINT8, 8, 6 * 4, MATRIX_LAYOUT_COLUMN_MAJOR> Matrix = {
      Buf, 0, 0};

  // PREVIEW CHECK TODO:
  // expected-error@+1{{something about transposing not supported for rowmajor / colmajor layouts}}
  return Mul<float>(Matrix,
                    MakeInterpretedVector<DATA_TYPE_UINT8_T4_PACKED>(Input));
}
