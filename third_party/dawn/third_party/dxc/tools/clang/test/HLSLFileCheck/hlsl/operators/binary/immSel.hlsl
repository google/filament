// RUN: %dxc -T ps_6_0 -O0 -E main %s | FileCheck %s

// Make sure use literal int is selected into i32 for value fit in i32.

// CHECK-NOT: or i64
// CHECK: or i32

uint main(const float4 P: A ) : SV_Target
{
    return ((P.x < -P.w) ? 3 : 9) | ((P.y < -P.z) ? 5 : 3);
}