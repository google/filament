// RUN: %dxc -E main -T ps_6_0 %s -Od /Zi | FileCheck %s

struct S {
  float x;
  float y;
};

float main() : SV_Target {
  // xHECK: %[[p_load:[0-9]+]] = load i32, i32*
  // xHECK-SAME: @dx.preserve.value
  // xHECK: %[[p:[0-9]+]] = trunc i32 %[[p_load]] to i1

  S a = { 0.f, 1.f};
  // xHECK: select i1 %[[p]], float
  // xHECK: select i1 %[[p]], float
  // CHECK: dx.nothing

  S b = { 2.f, 3.f};
  // xHECK: select i1 %[[p]], float
  // xHECK: select i1 %[[p]], float
  // CHECK: dx.nothing

  S c = { a.x+b.x, a.y+b.y };
  // CHECK: fmul
  // CHECK: fmul

  S d = c;
  // Memcpy should just get lowered to a noop for now.
  // CHECK: load i32, i32*
  // CHECK-SAME: @dx.nothing

  return d.x + d.y;
}
