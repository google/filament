// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: storeOutput.f32
// CHECK: float 5.000000
void fn(inout uint a) { a += 2; }

float4 main(): SV_TARGET {
uint2 u2 = 0;
uint2x2 mat2 = 1;
float m = 0;
fn(u2.x);
m += u2.x;
fn(mat2._11);
m += mat2._11;
return m;
}
