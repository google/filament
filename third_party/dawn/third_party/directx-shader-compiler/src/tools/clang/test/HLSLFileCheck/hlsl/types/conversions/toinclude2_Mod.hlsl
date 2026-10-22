// RUN: %dxc -E includedFunc2 -T ps_6_0 %s | FileCheck %s

// CHECK: @includedFunc2
float includedFunc2(int c : A) : SV_Target {
  return c + 100;
}