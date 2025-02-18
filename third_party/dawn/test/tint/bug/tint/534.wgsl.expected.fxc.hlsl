void set_vector_element(inout uint4 vec, int idx, uint val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

uint4 tint_ftou(float4 v) {
  return ((v <= (4294967040.0f).xxxx) ? ((v < (0.0f).xxxx) ? (0u).xxxx : uint4(v)) : (4294967295u).xxxx);
}

Texture2D<float4> src : register(t0);
Texture2D<float4> tint_symbol : register(t1);
RWByteAddressBuffer output : register(u2);
cbuffer cbuffer_uniforms : register(b3) {
  uint4 uniforms[1];
};

uint ConvertToFp16FloatValue(float fp32) {
  return 1u;
}

struct tint_symbol_3 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

void main_inner(uint3 GlobalInvocationID) {
  uint2 tint_tmp;
  src.GetDimensions(tint_tmp.x, tint_tmp.y);
  uint2 size = tint_tmp;
  uint2 dstTexCoord = GlobalInvocationID.xy;
  uint2 srcTexCoord = dstTexCoord;
  if ((uniforms[0].x == 1u)) {
    srcTexCoord.y = ((size.y - dstTexCoord.y) - 1u);
  }
  float4 srcColor = src.Load(uint3(srcTexCoord, uint(0)));
  float4 dstColor = tint_symbol.Load(uint3(dstTexCoord, uint(0)));
  bool success = true;
  uint4 srcColorBits = uint4(0u, 0u, 0u, 0u);
  uint4 dstColorBits = tint_ftou(dstColor);
  {
    for(uint i = 0u; (i < uniforms[0].w); i = (i + 1u)) {
      uint tint_symbol_1 = i;
      set_vector_element(srcColorBits, min(tint_symbol_1, 3u), ConvertToFp16FloatValue(srcColor[min(i, 3u)]));
      bool tint_tmp_1 = success;
      if (tint_tmp_1) {
        tint_tmp_1 = (srcColorBits[min(i, 3u)] == dstColorBits[min(i, 3u)]);
      }
      success = (tint_tmp_1);
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * uint(size.x)) + GlobalInvocationID.x);
  if (success) {
    uint tint_symbol_5 = 0u;
    output.GetDimensions(tint_symbol_5);
    uint tint_symbol_6 = ((tint_symbol_5 - 0u) / 4u);
    output.Store((4u * min(outputIndex, (tint_symbol_6 - 1u))), asuint(1u));
  } else {
    uint tint_symbol_7 = 0u;
    output.GetDimensions(tint_symbol_7);
    uint tint_symbol_8 = ((tint_symbol_7 - 0u) / 4u);
    output.Store((4u * min(outputIndex, (tint_symbol_8 - 1u))), asuint(0u));
  }
}

[numthreads(1, 1, 1)]
void main(tint_symbol_3 tint_symbol_2) {
  main_inner(tint_symbol_2.GlobalInvocationID);
  return;
}
