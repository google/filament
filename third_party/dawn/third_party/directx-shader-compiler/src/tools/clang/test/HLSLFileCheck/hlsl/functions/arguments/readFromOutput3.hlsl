// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 0
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 1
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 2
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 3
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 1, i8 0
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 1, i8 1
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 1, i8 2
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 1, i8 3

float c0;
float c1;
uint i;

struct VsIn {
   float4 a:A;
   float4  b:B;
};

struct VsOut {
   float4 pos: SV_Position;
   float4 c[2]: C;
};
void main(VsIn input, out VsOut output) {
  output.c[i%2] = input.b;
  output.c[i%2+1] = input.b;
 
  output.pos = output.c[i];
}
