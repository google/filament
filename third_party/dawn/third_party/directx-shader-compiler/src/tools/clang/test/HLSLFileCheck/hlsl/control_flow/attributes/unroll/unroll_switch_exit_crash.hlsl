// RUN: %dxc %s /T ps_6_0 | FileCheck %s

// Regression test for a crash when unrolling a loop whose exiting block
// uses a switch and not a branch.

// CHECK: call float @dx.op.unary.f32(i32 13
// CHECK: call float @dx.op.unary.f32(i32 13
// CHECK: call float @dx.op.unary.f32(i32 13
// CHECK: call float @dx.op.unary.f32(i32 13
// CHECK-NOT: call float @dx.op.unary.f32(i32 13

Texture1D<float> t0 : register(t0);

float foo(int cond) {
  float ret = 0;
  [unroll]
  for (int i = 0; i < 4; i++) {
    ret += 1;
    switch (cond) {
      case 0:
        ret += 10;
        break;
      case 1:
        ret += 20;
        break;
      default:
        return 42;
    }
    ret += sin(t0[cond + i]);
  }
  return ret;
}

[RootSignature("DescriptorTable(SRV(t0))")]
float main (int cond : COND) : SV_Target {
  float ret = 0;
  ret += foo(cond);
  return ret;
}
