// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s
// CHECK: @main

static const uint COUNT = 16;

float main() : SV_Target {
  float result = 10;
  int count = COUNT;
  [unroll]
  for(int i = count-1; i>=0; i--)
  {
    result += i;
  }

  return result;
}

