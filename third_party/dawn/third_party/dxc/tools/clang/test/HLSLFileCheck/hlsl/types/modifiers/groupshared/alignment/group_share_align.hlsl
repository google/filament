// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Make sure alignment is 4.
// CHECK:@{{.*}} = external addrspace(3) global [4 x float], align 4
// CHECK:store float {{.*}}, float addrspace(3)* {{.*}}, align 4
// CHECK:load float, float addrspace(3)* {{.*}}, align 4

groupshared float a[4];
RWBuffer<float> u;
[numthreads(8,8,1)]
void main(uint3 tid : SV_DispatchThreadID) {
  a[tid.x] = tid.y;
  GroupMemoryBarrier();
  u[tid.y] = a[tid.y];
}