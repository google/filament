// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

// Test that non-const arithmetic are not optimized away

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);

[RootSignature("DescriptorTable(SRV(t0), SRV(t1))")]
float4 main() : SV_Target {

  float2 xy = float2(10, 20);

  float2 zw = xy + float2(5, 30);
  // xHECK: fadd
  // xHECK: fadd
  // CHECK: @dx.nothing

  float2 foo = zw * 2;
  // xHECK: fmul
  // xHECK: fmul
  // CHECK: @dx.nothing

  float2 bar = foo / 0.5;
  // xHECK: fdiv
  // xHECK: fdiv
  // CHECK: @dx.nothing

  Texture2D tex = tex0; 
  // CHECK: br
  if (foo.x+bar.y >= 0) {
    tex = tex1;
    // CHECK: br
  }

  // CHECK: fadd
  // CHECK: fadd
  // CHECK: fadd
  // CHECK: fadd
  return tex.Load(0) + float4(foo,bar);
}

