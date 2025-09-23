// RUN: %dxc -I %hlsl_headers -T lib_6_9 %s -verify

#include <dx/linalg.h>
ByteAddressBuffer Buf;

export float4 Test1(vector<float, 4> Input) {
  using namespace dx::linalg;

  MatrixRef<DATA_TYPE_UINT16, 4, 4, MATRIX_LAYOUT_MUL_OPTIMAL, true> Matrix = {
      Buf, 0, 0};

  // expected-error@+3{{no matching function for call to 'MakeInterpretedVector'}}
  // expected-note@dx/linalg.h:113{{candidate template ignored: invalid explicitly-specified argument for template parameter 'DT'}}
  return Mul<float>(    
      Matrix, MakeInterpretedVector<2>(Input));
}

enum DataType {
  DATA_TYPE_InvalidType = 40
};

export float4 Test2(vector<float, 4> Input) {
  using namespace dx::linalg;

  MatrixRef<DATA_TYPE_UINT16, 4, 4, MATRIX_LAYOUT_MUL_OPTIMAL, true> Matrix = {
      Buf, 0, 0};

  // expected-error@+3{{no matching function for call to 'MakeInterpretedVector'}}
  // expected-note@dx/linalg.h:113{{candidate template ignored: invalid explicitly-specified argument for template parameter 'DT'}}
  return Mul<float>(    
      Matrix, MakeInterpretedVector<DATA_TYPE_InvalidType>(Input));
}

