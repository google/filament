// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Make sure alignment is 4.
// CHECK:@{{.*}} = addrspace(3) global [8 x float] undef, align 4
// CHECK:store float {{.*}}, float addrspace(3)* {{.*}}, align 4
// CHECK:store float {{.*}}, float addrspace(3)* {{.*}}, align 4
// CHECK:load float, float addrspace(3)* {{.*}}, align 4
// CHECK:load float, float addrspace(3)* {{.*}}, align 4

struct S {
   float2 a;
};

groupshared S a[4];
RWBuffer<float2> u;
[numthreads(8,8,1)]
void main(uint3 tid : SV_DispatchThreadID) {
  a[tid.x].a = tid.y;
  GroupMemoryBarrier();
  u[tid.y] = a[tid.y].a;
}