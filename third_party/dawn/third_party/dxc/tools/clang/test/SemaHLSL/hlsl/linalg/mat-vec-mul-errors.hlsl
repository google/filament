// RUN: %dxc -I %hlsl_headers -T lib_6_9 %s -verify

#include <dx/linalg.h>

ByteAddressBuffer Buf;

vector<float, 128> MixUpVectorAndMatrixArguments(vector<float, 128> Input) {
  using namespace dx::linalg;

  MatrixRef<DATA_TYPE_FLOAT16, 128, 128, MATRIX_LAYOUT_MUL_OPTIMAL> Matrix = {
      Buf, 0, 0};

  // expected-error@+2{{no matching function for call to 'Mul'}}
  // expected-note@dx/linalg.h:127{{candidate template ignored: could not match 'MatrixRefImpl' against 'InterpretedVector'}}
  return Mul<float>(MakeInterpretedVector<DATA_TYPE_FLOAT16>(Input), Matrix);
}
