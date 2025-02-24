struct Uniforms {
  float4x4 worldView;
  float4x4 proj;
  uint numPointLights;
  uint color_source;
  float4 color;
};

struct FragmentOutput {
  float4 color;
};

struct FragmentInput {
  float4 position;
  float4 view_position;
  float4 normal;
  float2 uv;
  float4 color;
};

struct main_outputs {
  float4 FragmentOutput_color : SV_Target0;
};

struct main_inputs {
  float4 FragmentInput_view_position : TEXCOORD0;
  float4 FragmentInput_normal : TEXCOORD1;
  float2 FragmentInput_uv : TEXCOORD2;
  float4 FragmentInput_color : TEXCOORD3;
  float4 FragmentInput_position : SV_Position;
};


cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[10];
};
ByteAddressBuffer pointLights : register(t1);
SamplerState mySampler : register(s2);
Texture2D<float4> myTexture : register(t3);
float4x4 v(uint start_byte_offset) {
  return float4x4(asfloat(uniforms[(start_byte_offset / 16u)]), asfloat(uniforms[((16u + start_byte_offset) / 16u)]), asfloat(uniforms[((32u + start_byte_offset) / 16u)]), asfloat(uniforms[((48u + start_byte_offset) / 16u)]));
}

Uniforms v_1(uint start_byte_offset) {
  float4x4 v_2 = v(start_byte_offset);
  float4x4 v_3 = v((64u + start_byte_offset));
  Uniforms v_4 = {v_2, v_3, uniforms[((128u + start_byte_offset) / 16u)][(((128u + start_byte_offset) % 16u) / 4u)], uniforms[((132u + start_byte_offset) / 16u)][(((132u + start_byte_offset) % 16u) / 4u)], asfloat(uniforms[((144u + start_byte_offset) / 16u)])};
  return v_4;
}

FragmentOutput main_inner(FragmentInput fragment) {
  FragmentOutput output = (FragmentOutput)0;
  output.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  v_1(0u);
  FragmentOutput v_5 = output;
  return v_5;
}

main_outputs main(main_inputs inputs) {
  FragmentInput v_6 = {float4(inputs.FragmentInput_position.xyz, (1.0f / inputs.FragmentInput_position.w)), inputs.FragmentInput_view_position, inputs.FragmentInput_normal, inputs.FragmentInput_uv, inputs.FragmentInput_color};
  FragmentOutput v_7 = main_inner(v_6);
  main_outputs v_8 = {v_7.color};
  return v_8;
}

