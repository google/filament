// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -HV 202x -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -HV 202x -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

RWByteAddressBuffer outbuf;

[numthreads(4,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // CHECK: call void @dx.op.linAlgVectorAccumulateToDescriptor.v4f32(i32 -2147483617, %dx.types.Handle %{{.*}}, i32 16,
  // CHECK-SAME: i32 64, <4 x float> <float 9.000000e+00, float 8.000000e+00, float 7.000000e+00, float 6.000000e+00>)
  // CHECK-SAME: ; LinAlgVectorAccumulateToDescriptor(handle,offset,align,vector)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32, <4 x float>)"
  // CHECK2-SAME: (i32 423, %dx.types.Handle %{{.*}}, i32 16, i32 64, <4 x float> %{{.*}})
  float4 vec = {9.0, 8.0, 7.0, 6.0};
  __builtin_LinAlg_VectorAccumulateToDescriptor(outbuf, 16, 64, vec);
}
