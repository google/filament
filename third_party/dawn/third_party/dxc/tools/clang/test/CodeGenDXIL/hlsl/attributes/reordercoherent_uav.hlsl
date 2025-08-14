// RUN: %dxc -E main -T lib_6_9 %s | FileCheck %s
// REQUIRES: dxil-1-9

// CHECK: !"uav1", {{.+}}, ![[TAGMD:[0-9]+]]}
// CHECK: ![[TAGMD]] = !{i32 0, i32 9, i32 4, i1 true

reordercoherent RWTexture1D<float4> uav1 : register(u3);
RWBuffer<float4> uav2;

[shader("raygeneration")]
void main()
{
  reordercoherent  RWTexture1D<float4> uav3 = uav1;
  uav3[0] = 5;
  uav1[0] = 2;
  uav2[1] = 3;
}
