// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

float foo(float arg) {
  return arg;
}

float main() : SV_Target {
  // xHECK: %[[p_load:[0-9]+]] = load i32, i32*
  // xHECK-SAME: @dx.preserve.value
  // xHECK: %[[p:[0-9]+]] = trunc i32 %[[p_load]] to i1

  float x = 10; // xHECK: %[[x:.+]] = select i1 %[[p]], float 1.000000e+01, float 1.000000e+01
  // CHECK: dx.nothing

  float y = foo(x); // CHECK: load i32, i32*
  // CHECK-SAME: @dx.nothing
    // Return
    // CHECK: load i32, i32*
    // CHECK-SAME: @dx.nothing
  // xHECK: %[[y:.+]] = select i1 %[[p]], float %[[x]], float %[[x]]
  // CHECK: dx.nothing

  return y;
}

