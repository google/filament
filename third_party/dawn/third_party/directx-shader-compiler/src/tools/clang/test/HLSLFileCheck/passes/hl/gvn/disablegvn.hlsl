// RUN: %dxc -opt-disable gvn -T ps_6_0 %s | FileCheck %s

// Simple test to verify disabling of gvn pass

// Verify that GVN is disabled by the presence
// of the second sin(), which GVN would have removed
// CHECK: call float @dx.op.unary.f32
// CHECK: call float @dx.op.unary.f32

float main(float a : A) : SV_Target {
  float res = sin(a);
  return res + sin(a);
}
