//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

float4 textureGatherCompare_144a9a() {
  float2 arg_2 = (1.0f).xx;
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  float4 res = arg_0.GatherCmp(arg_1, float3(arg_2, float(arg_3)), arg_4, int2((1).xx));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureGatherCompare_144a9a()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

float4 textureGatherCompare_144a9a() {
  float2 arg_2 = (1.0f).xx;
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  float4 res = arg_0.GatherCmp(arg_1, float3(arg_2, float(arg_3)), arg_4, int2((1).xx));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureGatherCompare_144a9a()));
  return;
}
//
// vertex_main
//
Texture2DArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

float4 textureGatherCompare_144a9a() {
  float2 arg_2 = (1.0f).xx;
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  float4 res = arg_0.GatherCmp(arg_1, float3(arg_2, float(arg_3)), arg_4, int2((1).xx));
  return res;
}

struct VertexOutput {
  float4 pos;
  float4 prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation float4 prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureGatherCompare_144a9a();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
