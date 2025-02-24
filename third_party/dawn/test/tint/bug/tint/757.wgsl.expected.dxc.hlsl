Texture2DArray<float4> myTexture : register(t1);
RWByteAddressBuffer result : register(u3);

struct tint_symbol_1 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

void main_inner(uint3 GlobalInvocationID) {
  uint flatIndex = (((4u * GlobalInvocationID.z) + (2u * GlobalInvocationID.y)) + GlobalInvocationID.x);
  flatIndex = (flatIndex * 1u);
  float4 texel = myTexture.Load(int4(int3(int2(GlobalInvocationID.xy), 0), 0));
  {
    for(uint i = 0u; (i < 1u); i = (i + 1u)) {
      uint tint_symbol_3 = 0u;
      result.GetDimensions(tint_symbol_3);
      uint tint_symbol_4 = ((tint_symbol_3 - 0u) / 4u);
      result.Store((4u * min((flatIndex + i), (tint_symbol_4 - 1u))), asuint(texel.r));
    }
  }
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.GlobalInvocationID);
  return;
}
