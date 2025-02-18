//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

float4 textureGatherCompare_2e409c() {
  float4 res = arg_0.GatherCmp(arg_1, float4((1.0f).xxx, float(1u)), 1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureGatherCompare_2e409c()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

float4 textureGatherCompare_2e409c() {
  float4 res = arg_0.GatherCmp(arg_1, float4((1.0f).xxx, float(1u)), 1.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureGatherCompare_2e409c()));
  return;
}
//
// vertex_main
//
TextureCubeArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

float4 textureGatherCompare_2e409c() {
  float4 res = arg_0.GatherCmp(arg_1, float4((1.0f).xxx, float(1u)), 1.0f);
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
  tint_symbol.prevent_dce = textureGatherCompare_2e409c();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
