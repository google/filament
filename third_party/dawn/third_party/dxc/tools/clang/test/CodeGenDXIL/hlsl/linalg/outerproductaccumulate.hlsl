// RUN: %dxc -I %hlsl_headers -T lib_6_9 -enable-16bit-types %s | FileCheck %s

#include <dx/linalg.h>

RWByteAddressBuffer RWBuf;

export void Test4(vector<half, 128> Input1, vector<half, 64> Input2) {
  using namespace dx::linalg;

  RWMatrixRef<DATA_TYPE_FLOAT16, 128, 64, MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL>
      matrix = {RWBuf, 0, 0};

  // CHECK: call void @dx.op.outerProductAccumulate.v128f16.v64f16(i32 307, <128 x half> %{{.+}}, <64 x half> %{{.+}}, %dx.types.Handle %{{.+}}, i32 0, i32 8, i32 3, i32 0)

  OuterProductAccumulate(Input1, Input2, matrix);  
}
