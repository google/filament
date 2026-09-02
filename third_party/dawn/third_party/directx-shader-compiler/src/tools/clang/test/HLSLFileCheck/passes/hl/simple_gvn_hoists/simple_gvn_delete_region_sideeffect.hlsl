// RUN: %dxc -T ps_6_0 -E main /opt-disable gvn %s | FileCheck %s

// CHECK: validation errors
// CHECK: Root Signature in DXIL container is not compatible with shader

// Make sure when there's side effect, the pass is not doing anything.

cbuffer cb : register(b6) {
  bool bCond;
};

cbuffer cb1 : register(b1) {
  float a, b, c;
};

RWTexture1D<float> uav0 : register(u0);

float foo(float2 uv) {
  uav0[0] = 10;
  return sin(a) + b + c;
}

float bar() {
  uav0[0] = 10;
  return sin(a) + b + c;
}

[RootSignature("CBV(b1),DescriptorTable(UAV(u0))")]
float main(float2 uv : TEXCOORD) : SV_Target {
  if (bCond) {
    return foo(uv);
  }
  else {
    return bar();
  }
}


