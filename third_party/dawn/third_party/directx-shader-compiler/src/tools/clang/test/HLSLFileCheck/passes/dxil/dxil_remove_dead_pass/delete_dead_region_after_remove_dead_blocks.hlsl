// RUN: %dxc %s -T ps_6_0 -Od | FileCheck %s

// CBuffer member 'foo' is used for a condition to decide the value of 'cond'.
// But 'cond' is eventually set to 0, under a compile time known condition 'my_cond'.
//
// The way the Od dead code removal passes were arranged was unable to clean up this pattern.
// EraseDeadRegion would run first, but unable to delete 'if (foo)' because of outflowing
// value 'cond' still being used.
//
// If we run RemoveDeadBlocks first, however, it would delete the branch `if (!my_cond)` and
// relieve the use of 'cond', allowing EraseDeadRegion to delete `if (foo)`.
//

// CHECK: @main
// CHECK-NOT: call %dx.types.CBufRet.f32 @dx.op.cbufferLoad

cbuffer cb : register(b0) {
  float foo;
  float bar;
}

[RootSignature("")]
float main(float i : I) : SV_Target {

  float cond = 0;
  if (foo) {
    cond = sin(i);
  }

  float my_cond = 0;
  if (!my_cond) {
    cond = 0;
  }

  return cond ? 10 : 20;
}
