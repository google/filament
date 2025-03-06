// RUN: %dxc -E main -T ps_6_6 %s -Od | FileCheck %s

// CHECK: local resource not guaranteed to map to unique global resource

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t42);

const float my_const = 0;

[RootSignature("CBV(b0), DescriptorTable(SRV(t0), SRV(t42))")]
float4 main(uint2 uv : TEXCOORD) : SV_Target {

  float val = my_const;

  Texture2D tex = tex0;
  if (val > 0) {
    tex = tex1;
  }
  return tex.Load(0);
}

