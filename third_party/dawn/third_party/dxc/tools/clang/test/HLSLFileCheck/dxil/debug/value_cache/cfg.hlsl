// RUN: %dxc -E main -T ps_6_0 -Od %s | FileCheck %s

// CHECK: @main

static bool gG;
static bool gG2;
static bool gG3;

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t42);
Texture2D tex2 : register(t2);

Texture2D f(bool foo) {
  if (foo)
    return tex0;
  else
    return tex1;
}

Texture2D h(bool foo3) {
  return foo3 ? f(gG2) : tex2;
}

[RootSignature("DescriptorTable(SRV(t0, numDescriptors=3), SRV(t42))")]
float4 main() : sv_target {

  // CHECK: %[[handle:.+]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 42

  gG = true;
  gG2 = false;
  gG3 = false;

  // CHECK: @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %[[handle]]
  return h(gG).Load(0);
};


