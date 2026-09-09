// RUN: %dxc -T lib_6_3 %s | FileCheck %s

// Verify no phi on resource, handle, or anything

// CHECK: define void @"\01?main{{[@$?.A-Za-z0-9_]+}}"()
// CHECK-NOT: phi
// CHECK: ret void


Texture2D<float4> Textures[2] : register(t0, space0);
RWTexture2D<float4> RWTextures[2] : register(u0, space0);

static float4 load(uint resIdx, uint2 texel) {
  if (((resIdx >> 8) & 3) == 1) {
    return Textures[resIdx & 0xFF].Load(uint3(texel, 0));
  }
  if (((resIdx >> 8) & 3) == 2) {
    return RWTextures[resIdx & 0xFF][texel];
  }
  return 0.0f;
}

static void store(uint resIdx, uint2 texel, float4 value) {
  if (((resIdx >> 8) & 3) == 2) {
    RWTextures[resIdx & 0xFF][texel] = value;
  }
  return;
}

RWTexture2D<float> OutBuf0 : register(u2, space0);

[shader("raygeneration")]
void main() {
  uint2 launchIdx = DispatchRaysIndex();
  const uint resIdx = 0x200;
  float4 value = load(resIdx, launchIdx);
  if (value.x > 10.0f) {
    OutBuf0[launchIdx] = 0.0f;
  }
  store(resIdx, launchIdx, value);
  return;
}
