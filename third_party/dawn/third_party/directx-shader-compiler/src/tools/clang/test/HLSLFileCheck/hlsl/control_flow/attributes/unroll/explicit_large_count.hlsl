// RUN: %dxc -Od -E main -T ps_6_0 %s | FileCheck %s
// CHECK: @main

// Confirm that the 128 limit on loop unroll can be overritten by an explicit
// loop count

[RootSignature("")]
float main(float y : Y) : SV_Target {
  float x = 0;

  static const uint kLoopCount = 512;

  [unroll(kLoopCount)]
  for (uint i = 0; i < kLoopCount; ++i)
  {
    x = x * x + y;
  }
  return x;
}

