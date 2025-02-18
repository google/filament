//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<float4> arg_0 : register(t0, space1);

float4 textureLoad_65a4d0() {
  float4 res = arg_0.Load(uint4(uint3((1u).xx, uint(1)), uint(0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_65a4d0()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<float4> arg_0 : register(t0, space1);

float4 textureLoad_65a4d0() {
  float4 res = arg_0.Load(uint4(uint3((1u).xx, uint(1)), uint(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_65a4d0()));
  return;
}
//
// vertex_main
//
Texture2DArray<float4> arg_0 : register(t0, space1);

float4 textureLoad_65a4d0() {
  float4 res = arg_0.Load(uint4(uint3((1u).xx, uint(1)), uint(0)));
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
  tint_symbol.prevent_dce = textureLoad_65a4d0();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
