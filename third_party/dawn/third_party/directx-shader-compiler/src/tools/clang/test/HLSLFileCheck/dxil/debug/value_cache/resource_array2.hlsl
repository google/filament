// RUN: %dxc -E main -T ps_6_0 -Od %s | FileCheck %s

// Make sure unused resources are handled even when
// they're arrays.

// CHECK: @main

static bool gG;
static bool gG2;
static bool gG3;

Texture2D tex0[42] : register(t0);
Texture2D tex1 : register(t42);
Texture2D tex2[2][2] : register(t43);

const uint idx1;
const uint idx2;
const uint idx3;

static Texture2D local_tex1;
static Texture2D local_tex2;
static Texture2D local_tex3;

Texture2D f(bool foo) {
  [branch]
  if (foo)
    return local_tex1;
  else
    return tex1;
}
Texture2D g(bool foo) {
  [branch]
  if (foo)
    return local_tex2;
  else
    return local_tex3;
}

Texture2D h(bool foo3) {
  return foo3 ? f(gG2) : g(gG3);
}

[RootSignature("CBV(b0), DescriptorTable(SRV(t0, numDescriptors=42), SRV(t42), SRV(t43, numDescriptors=4))")]
float4 main() : sv_target {
  // CHECK: %[[handle:.+]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 42

  local_tex1 = tex0[idx1];
  local_tex2 = tex0[idx2];
  local_tex3 = tex2[idx3][idx1];

  gG = true;
  gG2 = false;
  gG3 = false;

  // CHECK: @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %[[handle]]
  return h(gG).Load(0);
};


