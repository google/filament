// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK-NOT: dx.op.loadInput.f32(i32 4, i32 0

float c0;
float c1;

struct PsIn {
   float4 a:A;
   float4  b:B;
};

float4 main(PsIn input) : SV_TARGET {
  input.a = c0;
  return input.a+c1+input.b;
}
