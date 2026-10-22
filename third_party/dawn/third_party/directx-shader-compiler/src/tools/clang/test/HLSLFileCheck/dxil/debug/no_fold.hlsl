// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

// Test that non-const arithmetic are not optimized away

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);

[RootSignature("DescriptorTable(SRV(t0), SRV(t1))")]
float4 main() : SV_Target {

  float x = 10;

  float y = x + 5;
  // xHECK: fadd
  // CHECK: @dx.nothing

  float z = y * 2;
  // xHECK: fmul
  // CHECK: @dx.nothing

  float w = z / 0.5;
  // xHECK: fdiv
  // CHECK: @dx.nothing

  Texture2D tex = tex0; 
  // CHECK: br
  if (w >= 0) {
    tex = tex1;
    // CHECK: br
  }

  // CHECK: fadd
  // CHECK: fadd
  // CHECK: fadd
  // CHECK: fadd
  return tex.Load(0) + float4(x,y,z,w);
}

