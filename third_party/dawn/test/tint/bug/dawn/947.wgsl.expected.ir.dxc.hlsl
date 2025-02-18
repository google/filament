//
// vs_main
//
struct VertexOutputs {
  float2 texcoords;
  float4 position;
};

struct vs_main_outputs {
  float2 VertexOutputs_texcoords : TEXCOORD0;
  float4 VertexOutputs_position : SV_Position;
};

struct vs_main_inputs {
  uint VertexIndex : SV_VertexID;
};


cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[1];
};
VertexOutputs vs_main_inner(uint VertexIndex) {
  float2 texcoord[3] = {float2(-0.5f, 0.0f), float2(1.5f, 0.0f), float2(0.5f, 2.0f)};
  VertexOutputs output = (VertexOutputs)0;
  output.position = float4(((texcoord[min(VertexIndex, 2u)] * 2.0f) - (1.0f).xx), 0.0f, 1.0f);
  bool flipY = (asfloat(uniforms[0u].y) < 0.0f);
  if (flipY) {
    output.texcoords = ((((texcoord[min(VertexIndex, 2u)] * asfloat(uniforms[0u].xy)) + asfloat(uniforms[0u].zw)) * float2(1.0f, -1.0f)) + float2(0.0f, 1.0f));
  } else {
    output.texcoords = ((((texcoord[min(VertexIndex, 2u)] * float2(1.0f, -1.0f)) + float2(0.0f, 1.0f)) * asfloat(uniforms[0u].xy)) + asfloat(uniforms[0u].zw));
  }
  VertexOutputs v = output;
  return v;
}

vs_main_outputs vs_main(vs_main_inputs inputs) {
  VertexOutputs v_1 = vs_main_inner(inputs.VertexIndex);
  vs_main_outputs v_2 = {v_1.texcoords, v_1.position};
  return v_2;
}

//
// fs_main
//
struct fs_main_outputs {
  float4 tint_symbol : SV_Target0;
};

struct fs_main_inputs {
  float2 texcoord : TEXCOORD0;
};


float4 fs_main_inner(float2 texcoord) {
  float2 clampedTexcoord = clamp(texcoord, (0.0f).xx, (1.0f).xx);
  if (!(all((clampedTexcoord == texcoord)))) {
    discard;
  }
  float4 srcColor = (0.0f).xxxx;
  return srcColor;
}

fs_main_outputs fs_main(fs_main_inputs inputs) {
  fs_main_outputs v = {fs_main_inner(inputs.texcoord)};
  return v;
}

