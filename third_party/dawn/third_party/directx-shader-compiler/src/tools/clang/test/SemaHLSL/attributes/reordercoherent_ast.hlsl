// RUN: %dxc -T lib_6_9 -ast-dump %s | FileCheck %s
// REQUIRES: dxil-1-9

// CHECK: |-VarDecl {{.*}} used uav1 'reordercoherent RWTexture1D<float4>':'RWTexture1D<vector<float, 4> >'
// CHECK-NEXT: | |-HLSLReorderCoherentAttr
reordercoherent RWTexture1D<float4> uav1 : register(u3);
RWBuffer<float4> uav2;

[shader("raygeneration")]
void main()
{
 // CHECK: |   `-VarDecl {{.*}} uav3 'reordercoherent RWTexture1D<float4>':'RWTexture1D<vector<float, 4> >' cinit
 // CHECK-NEXT: |     |
 // CHECK-NEXT: |     |
 // CHECK-NEXT: |     `-HLSLReorderCoherentAttr
  reordercoherent  RWTexture1D<float4> uav3 = uav1;
}
