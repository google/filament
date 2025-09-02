// RUN: %dxc -I %hlsl_headers -T lib_6_9 %s | FileCheck %s

#include <dx/linalg.h>

RWByteAddressBuffer RWBuf;

export void Test5(vector<half, 128> Input) {
  using namespace dx::linalg;

  RWBuf.Store<vector<half, 128> >(0, Input);

  // CHECK: call void @dx.op.vectorAccumulate.v128f32(i32 308, <128 x float> %{{.*}}, %dx.types.Handle %{{.*}}, i32 0)
  VectorAccumulate(Input, RWBuf, 0);
}
