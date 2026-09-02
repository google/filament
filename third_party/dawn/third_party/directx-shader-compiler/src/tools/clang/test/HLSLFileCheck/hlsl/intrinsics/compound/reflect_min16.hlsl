// RUN: %dxc -T ps_6_0 %s | FileCheck %s

// Ensure no validation errors
// CHECK-NOT: error

// Ensure the reflect generated intrinsics are present
// CHECK: call half @dx.op.dot3.f16
// CHECK: fmul fast half {{.*}}, 0xH4000


min16float3 main(min16float3 i : I, min16float3 n: N) : SV_Target {
  return reflect(i,n);
}
