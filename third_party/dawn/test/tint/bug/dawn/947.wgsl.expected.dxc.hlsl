//
// vs_main
//
cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[1];
};

struct VertexOutputs {
  float2 texcoords;
  float4 position;
};
struct tint_symbol_1 {
  uint VertexIndex : SV_VertexID;
};
struct tint_symbol_2 {
  float2 texcoords : TEXCOORD0;
  float4 position : SV_Position;
};

VertexOutputs vs_main_inner(uint VertexIndex) {
  float2 texcoord[3] = {float2(-0.5f, 0.0f), float2(1.5f, 0.0f), float2(0.5f, 2.0f)};
  VertexOutputs output = (VertexOutputs)0;
  output.position = float4(((texcoord[min(VertexIndex, 2u)] * 2.0f) - (1.0f).xx), 0.0f, 1.0f);
  bool flipY = (asfloat(uniforms[0].y) < 0.0f);
  if (flipY) {
    output.texcoords = ((((texcoord[min(VertexIndex, 2u)] * asfloat(uniforms[0].xy)) + asfloat(uniforms[0].zw)) * float2(1.0f, -1.0f)) + float2(0.0f, 1.0f));
  } else {
    output.texcoords = ((((texcoord[min(VertexIndex, 2u)] * float2(1.0f, -1.0f)) + float2(0.0f, 1.0f)) * asfloat(uniforms[0].xy)) + asfloat(uniforms[0].zw));
  }
  return output;
}

tint_symbol_2 vs_main(tint_symbol_1 tint_symbol) {
  VertexOutputs inner_result = vs_main_inner(tint_symbol.VertexIndex);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.texcoords = inner_result.texcoords;
  wrapper_result.position = inner_result.position;
  return wrapper_result;
}
//
// fs_main
//
struct tint_symbol_1 {
  float2 texcoord : TEXCOORD0;
};
struct tint_symbol_2 {
  float4 value : SV_Target0;
};

float4 fs_main_inner(float2 texcoord) {
  if (true) {
    float2 clampedTexcoord = clamp(texcoord, (0.0f).xx, (1.0f).xx);
    if (!(all((clampedTexcoord == texcoord)))) {
      discard;
    }
    float4 srcColor = (0.0f).xxxx;
    return srcColor;
  }
  float4 unused;
  return unused;
}

tint_symbol_2 fs_main(tint_symbol_1 tint_symbol) {
  float4 inner_result = fs_main_inner(tint_symbol.texcoord);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
