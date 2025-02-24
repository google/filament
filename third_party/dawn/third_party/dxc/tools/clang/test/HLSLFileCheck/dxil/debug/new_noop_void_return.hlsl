// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

static float my_glob;

void foo() {
  my_glob = 10;
  return;
}

[RootSignature("")]
float main() : SV_Target {
  // xHECK: %[[p_load:[0-9]+]] = load i32, i32*
  // xHECK-SAME: @dx.preserve.value
  // xHECK: %[[p:[0-9]+]] = trunc i32 %[[p_load]] to i1

  // xHECK: select i1 %[[p]]
  // CHECK: dx.nothing
  my_glob = 0;

  // Function call
  // CHECK: load i32, i32*
  // CHECK: @dx.nothing
  foo();
    // xHECK: select i1 %[[p]]
    // CHECK: dx.nothing
    // void return

  return my_glob;
}



