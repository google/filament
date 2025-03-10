// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

// Make sure DxilValueCache actually predicts the correct branch

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t42);

[RootSignature("DescriptorTable(SRV(t0), SRV(t42))")]
float4 main() : SV_Target {
  // CHECK: %[[handle:.+]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 42

  float x = 10;
  float y = x + 5;
  float z = y * 2;
  float w = z / 0.5;

  Texture2D tex = tex0; 

  if (w >= 0) {
    tex = tex1;
  }
  // CHECK: @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %[[handle]]
  return tex.Load(0) + float4(x,y,z,w);
}

