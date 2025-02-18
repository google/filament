cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[10];
};
ByteAddressBuffer pointLights : register(t1);
SamplerState mySampler : register(s2);
Texture2D<float4> myTexture : register(t3);

struct FragmentInput {
  float4 position;
  float4 view_position;
  float4 normal;
  float2 uv;
  float4 color;
};
struct FragmentOutput {
  float4 color;
};
struct tint_symbol_1 {
  float4 view_position : TEXCOORD0;
  float4 normal : TEXCOORD1;
  float2 uv : TEXCOORD2;
  float4 color : TEXCOORD3;
  float4 position : SV_Position;
};
struct tint_symbol_2 {
  float4 color : SV_Target0;
};

FragmentOutput main_inner(FragmentInput fragment) {
  FragmentOutput output = (FragmentOutput)0;
  output.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  return output;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  FragmentInput tint_symbol_3 = {float4(tint_symbol.position.xyz, (1.0f / tint_symbol.position.w)), tint_symbol.view_position, tint_symbol.normal, tint_symbol.uv, tint_symbol.color};
  FragmentOutput inner_result = main_inner(tint_symbol_3);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.color = inner_result.color;
  return wrapper_result;
}
