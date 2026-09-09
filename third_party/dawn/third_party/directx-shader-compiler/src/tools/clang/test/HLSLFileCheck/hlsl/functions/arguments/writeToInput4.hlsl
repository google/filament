// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure the input is used.
// CHECK: call float @dx.op.loadInput.f32

struct Input {
    float a : A;
    float b : B;
};

Input ci;

float4 main(Input i) : SV_Target {
   float c = i.a + i.b;
   i = ci;
   c += i.a * i.b;
   return c;
}