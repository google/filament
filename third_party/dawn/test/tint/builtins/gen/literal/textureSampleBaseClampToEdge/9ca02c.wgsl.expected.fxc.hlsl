//
// fragment_main
//
float4 tint_textureSampleBaseClampToEdge(Texture2D<float4> t, SamplerState s, float2 coord) {
  uint3 tint_tmp;
  t.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  float2 dims = float2(tint_tmp.xy);
  float2 half_texel = ((0.5f).xx / dims);
  float2 clamped = clamp(coord, half_texel, (1.0f - half_texel));
  return t.SampleLevel(s, clamped, 0.0f);
}

RWByteAddressBuffer prevent_dce : register(u0);
Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

float4 textureSampleBaseClampToEdge_9ca02c() {
  float4 res = tint_textureSampleBaseClampToEdge(arg_0, arg_1, (1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBaseClampToEdge_9ca02c()));
  return;
}
//
// compute_main
//
float4 tint_textureSampleBaseClampToEdge(Texture2D<float4> t, SamplerState s, float2 coord) {
  uint3 tint_tmp;
  t.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  float2 dims = float2(tint_tmp.xy);
  float2 half_texel = ((0.5f).xx / dims);
  float2 clamped = clamp(coord, half_texel, (1.0f - half_texel));
  return t.SampleLevel(s, clamped, 0.0f);
}

RWByteAddressBuffer prevent_dce : register(u0);
Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

float4 textureSampleBaseClampToEdge_9ca02c() {
  float4 res = tint_textureSampleBaseClampToEdge(arg_0, arg_1, (1.0f).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBaseClampToEdge_9ca02c()));
  return;
}
//
// vertex_main
//
float4 tint_textureSampleBaseClampToEdge(Texture2D<float4> t, SamplerState s, float2 coord) {
  uint3 tint_tmp;
  t.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  float2 dims = float2(tint_tmp.xy);
  float2 half_texel = ((0.5f).xx / dims);
  float2 clamped = clamp(coord, half_texel, (1.0f - half_texel));
  return t.SampleLevel(s, clamped, 0.0f);
}

Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

float4 textureSampleBaseClampToEdge_9ca02c() {
  float4 res = tint_textureSampleBaseClampToEdge(arg_0, arg_1, (1.0f).xx);
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
  tint_symbol.prevent_dce = textureSampleBaseClampToEdge_9ca02c();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
