// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0
// CHECK: dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1
// CHECK: dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2
// CHECK: dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 3
// CHECK: dx.op.loadInput.f32(i32 4, i32 0, i32 1, i8 0
// CHECK: dx.op.loadInput.f32(i32 4, i32 0, i32 1, i8 1
// CHECK: dx.op.loadInput.f32(i32 4, i32 0, i32 1, i8 2
// CHECK: dx.op.loadInput.f32(i32 4, i32 0, i32 1, i8 3
// CHECK-NOT: dx.op.loadInput.f32(i32 4, i32 0,


float c0;
float c1;
uint i;
struct PsIn {
   float4 a[2]:A;
   float4  b:B;
};

float4 main(PsIn input) : SV_TARGET {
  input.a[i%2] = c0;
  input.a[i%2+1] = c1;
  return input.a[0]+input.b;
}
