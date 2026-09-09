// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

Texture2D tex0[42] : register(t0);
Texture2D tex1 : register(t42);

const static float2 my_offsets[] = {
  float2(1,2),
  float2(3,4),
  float2(5,6),
  float2(7,8),
};

[RootSignature("DescriptorTable(SRV(t0, numDescriptors=42), SRV(t42)), DescriptorTable(Sampler(s0))")]
float4 main(uint2 uv : TEXCOORD, uint a : A) : SV_Target {
  // CHECK: %[[handle:.+]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 42
  int x = 0;
  int y = 0;

  float2 val = my_offsets[x+y+1];

  Texture2D tex = tex0[a];
  if (val.x > 0) {
    tex = tex1;
  }
  // CHECK: @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %[[handle]]
  return tex.Load(0);
}

