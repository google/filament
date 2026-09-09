// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: 64-Bit integer
// CHECK: add i64
// CHECK: udiv i64
// CHECK: shl i64
// CHECK: mul i64
// CHECK: UMax
// CHECK: UMin
// CHECK: uitofp i64


struct Foo
{
  uint64_t b;
};

StructuredBuffer<Foo> buf1;
RWStructuredBuffer<Foo> buf2;

uint64_t4x3 u;
uint64_t a;
int64_t b;
float4 main(float idx1 : Idx1, float idx2 : Idx2, int2 c : C) : SV_Target
{
  uint64_t4 r = buf1.Load(idx1).b + a;
  r += u[0].xyzz;
  r /= u._m01_m11_m21_m22;

  buf2[idx1*3].b = r;

  r *= b << 5;
  r = abs(r); // No-op on uints
  r = max(r, c.x);
  r = min(r, c.y);
  return r;
}