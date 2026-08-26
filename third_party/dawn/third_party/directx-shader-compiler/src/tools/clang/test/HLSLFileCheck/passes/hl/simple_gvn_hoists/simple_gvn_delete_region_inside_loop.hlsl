// RUN: %dxc -T ps_6_0 -E main /opt-disable gvn %s | FileCheck %s

// CHECK: @main
// CHECK: call float @dx.op.unary.f32(i32 13
// CHECK-NOT: call float @dx.op.unary.f32(i32 13
// ^Make sure sin is only seen once

// Without GVN, some basic ability to recognize equivalent regions was lost.
// This test makes sure we can still eliminate redundancies even when regular GVN
// is disabled to reduce code motion.
// 
// In this regression test, foo and bar do the exact same thing. The branch that
// decides which one to call reads from a resource not bound in root signature.
// The compiler should be able to recognize that both sides are equivalent and
// eliminate the condition even when gvn is turned off.
//

cbuffer cb : register(b6) {
  bool bCond;
};

cbuffer cb1 : register(b1) {
  float a, b, c;
};

float foo(float2 uv) {
  return sin(a) + b + c;
}

float bar() {
  return sin(a) + b + c;
}

[RootSignature("CBV(b1)")]
float main(float2 uv : TEXCOORD, int loopBound : LOOPBOUND) : SV_Target {
    float ret = 0;
    [loop]
    for (int i = 0; i < loopBound; i++) {
        if (bCond) {
            ret += foo(uv);
        }
        else {
            ret += bar();
        }
    }
    return ret;
}


