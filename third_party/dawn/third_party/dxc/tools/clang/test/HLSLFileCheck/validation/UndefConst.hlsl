// RUN: %dxc -T ps_6_0 %s | FileCheck %s

// A warning should be produced, but isn't at present
// CHXXXCK: Instructions should not read uninitialized value

// For now, just check for no crash
// CHECK: @main

float main() : SV_Target {
  const float a;
  return a;
}
