// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
static float f1 = { true };

static bool f2 = { 1 };

static int f3 = { false };

float4 main() : SV_TARGET {
  return f1 + f2 + f3;
}
