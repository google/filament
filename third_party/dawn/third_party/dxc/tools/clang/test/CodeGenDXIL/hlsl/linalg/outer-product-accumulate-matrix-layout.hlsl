// RUN: %dxc -I %hlsl_headers -T cs_6_9 %s -enable-16bit-types -DML=MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL -DSTRIDE=0 2>&1 | FileCheck %s

//Source file for the IR in \tools\clang\test\LitDXILValidation\outer-product-accumulate-matrix-layout-failing.ll
//Source file for the IR in \tools\clang\test\LitDXILValidation\outer-product-accumulate-matrix-layout-passing.ll

ByteAddressBuffer input_vector_buffer;
ByteAddressBuffer input_vector_buffer2;
RWByteAddressBuffer matrix_buffer;

#include <dx/linalg.h>

// CHECK: call void @dx.op.outerProductAccumulate.v8f16.v8f16(i32 307, <8 x half> %{{[^ ]+}}, <8 x half> %{{[^ ]+}}, %dx.types.Handle %{{[^ ]+}}, i32 0, i32 8, i32 3, i32 0)
using namespace dx::linalg;

[Numthreads(1,1,1)]
[shader("compute")]
void main()
{
  vector<half, 8> input_vector1 = input_vector_buffer.Load<vector<half, 8> >(0);
  vector<half, 8> input_vector2 = input_vector_buffer2.Load<vector<half, 8> >(0);

  const uint matrix_interpretation = DATA_TYPE_FLOAT16;
  const uint matrix_layout = ML;
  const uint matrix_offset = 0;
  const uint matrix_stride = STRIDE;

  __builtin_OuterProductAccumulate(input_vector1, input_vector2, matrix_buffer, matrix_offset, matrix_interpretation, matrix_layout, matrix_stride);
}
