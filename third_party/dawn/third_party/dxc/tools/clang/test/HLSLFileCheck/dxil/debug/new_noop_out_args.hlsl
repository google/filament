// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

struct S {
  float x;
  float y;
};

void foo(out S arg) {
  arg.x = 20;
  arg.y = 30;
  return;
}

void bar(inout S arg) {
  arg.x *= 2;
  arg.y *= 3;
  return;
}

void baz(inout float x, inout float y) {
  x *= 0.5;
  y *= 0.5;
  return;
}

[RootSignature("")]
float main() : SV_Target {
  // xHECK: %[[p_load:[0-9]+]] = load i32, i32*
  // xHECK-SAME: @dx.preserve.value
  // xHECK: %[[p:[0-9]+]] = trunc i32 %[[p_load]] to i1

  S s;

  // CHECK: load i32, i32*
  // CHECK: @dx.nothing
  foo(s);
    // xHECK: select i1 %[[p]]
    // xHECK: select i1 %[[p]]
    // CHECK: dx.nothing
    // CHECK: load i32, i32*
    // CHECK: @dx.nothing

  // CHECK: load i32, i32*
  // CHECK: @dx.nothing
  bar(s);
    // xHECK: fmul
    // xHECK: fmul
    // CHECK: load i32, i32*
    // CHECK: @dx.nothing

  // CHECK: load i32, i32*
  // CHECK: @dx.nothing
  baz(s.x, s.y);
    // xHECK: fmul
    // xHECK: fmul
    // CHECK: load i32, i32*
    // CHECK: @dx.nothing

  // xHECK: fadd
  return s.x + s.y;
}



