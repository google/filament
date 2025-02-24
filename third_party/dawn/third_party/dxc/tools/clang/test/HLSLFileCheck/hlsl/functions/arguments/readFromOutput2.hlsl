// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: fadd

float c0;
float c1;

struct VsIn {
   float4 a:A;
   float4  b:B;
};

struct VsOut {
   float4 pos: SV_Position;
   float4 c: C;
};
void main(VsIn input, out VsOut output) {
  output.pos = input.a;
  output.c = input.b + output.pos;
}
