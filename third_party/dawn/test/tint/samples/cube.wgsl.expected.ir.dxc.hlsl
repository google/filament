//
// vtx_main
//
struct VertexOutput {
  float4 vtxFragColor;
  float4 Position;
};

struct VertexInput {
  float4 cur_position;
  float4 color;
};

struct vtx_main_outputs {
  float4 VertexOutput_vtxFragColor : TEXCOORD0;
  float4 VertexOutput_Position : SV_Position;
};

struct vtx_main_inputs {
  float4 VertexInput_cur_position : TEXCOORD0;
  float4 VertexInput_color : TEXCOORD1;
};


cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[4];
};
float4x4 v(uint start_byte_offset) {
  return float4x4(asfloat(uniforms[(start_byte_offset / 16u)]), asfloat(uniforms[((16u + start_byte_offset) / 16u)]), asfloat(uniforms[((32u + start_byte_offset) / 16u)]), asfloat(uniforms[((48u + start_byte_offset) / 16u)]));
}

VertexOutput vtx_main_inner(VertexInput input) {
  VertexOutput output = (VertexOutput)0;
  output.Position = mul(input.cur_position, v(0u));
  output.vtxFragColor = input.color;
  VertexOutput v_1 = output;
  return v_1;
}

vtx_main_outputs vtx_main(vtx_main_inputs inputs) {
  VertexInput v_2 = {inputs.VertexInput_cur_position, inputs.VertexInput_color};
  VertexOutput v_3 = vtx_main_inner(v_2);
  vtx_main_outputs v_4 = {v_3.vtxFragColor, v_3.Position};
  return v_4;
}

//
// frag_main
//
struct frag_main_outputs {
  float4 tint_symbol : SV_Target0;
};

struct frag_main_inputs {
  float4 fragColor : TEXCOORD0;
};


float4 frag_main_inner(float4 fragColor) {
  return fragColor;
}

frag_main_outputs frag_main(frag_main_inputs inputs) {
  frag_main_outputs v = {frag_main_inner(inputs.fragColor)};
  return v;
}

