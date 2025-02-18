//
// vtx_main
//
cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[4];
};

struct VertexInput {
  float4 cur_position;
  float4 color;
};
struct VertexOutput {
  float4 vtxFragColor;
  float4 Position;
};
struct tint_symbol_1 {
  float4 cur_position : TEXCOORD0;
  float4 color : TEXCOORD1;
};
struct tint_symbol_2 {
  float4 vtxFragColor : TEXCOORD0;
  float4 Position : SV_Position;
};

float4x4 uniforms_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(uniforms[scalar_offset / 4]), asfloat(uniforms[scalar_offset_1 / 4]), asfloat(uniforms[scalar_offset_2 / 4]), asfloat(uniforms[scalar_offset_3 / 4]));
}

VertexOutput vtx_main_inner(VertexInput input) {
  VertexOutput output = (VertexOutput)0;
  output.Position = mul(input.cur_position, uniforms_load(0u));
  output.vtxFragColor = input.color;
  return output;
}

tint_symbol_2 vtx_main(tint_symbol_1 tint_symbol) {
  VertexInput tint_symbol_3 = {tint_symbol.cur_position, tint_symbol.color};
  VertexOutput inner_result = vtx_main_inner(tint_symbol_3);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.vtxFragColor = inner_result.vtxFragColor;
  wrapper_result.Position = inner_result.Position;
  return wrapper_result;
}
//
// frag_main
//
struct tint_symbol_1 {
  float4 fragColor : TEXCOORD0;
};
struct tint_symbol_2 {
  float4 value : SV_Target0;
};

float4 frag_main_inner(float4 fragColor) {
  return fragColor;
}

tint_symbol_2 frag_main(tint_symbol_1 tint_symbol) {
  float4 inner_result = frag_main_inner(tint_symbol.fragColor);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
