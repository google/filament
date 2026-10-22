// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: %"hostlayout.$Globals" = type { float, %hostlayout.struct.S, <4 x i32> }
// CHECK: %hostlayout.struct.S = type { i32, i32, i32, <2 x i32>, i32, i32, i32 }

RasterizerOrderedBuffer<float4> r;

min16float min16float_val;
struct S {
bool t;
bool t3;
int  m2;
bool2 t2;
int  m;
min16int i;
int  x;
};

S s;
bool4x1 b;

float4 main(float4 p: SV_Position) : SV_Target {
  min16float min16float_local = min16float_val;
  if (min16float_local != 0) {
    return 1;
  }
  return r[0] + b[0] + s.m + s.x;
}
