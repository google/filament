//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMS<float4> arg_0 : register(t0, space1);

float textureLoad_fcd23d() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = 1;
  float res = arg_0.Load(uint3(arg_1, uint(0)), arg_2).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureLoad_fcd23d()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMS<float4> arg_0 : register(t0, space1);

float textureLoad_fcd23d() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = 1;
  float res = arg_0.Load(uint3(arg_1, uint(0)), arg_2).x;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(textureLoad_fcd23d()));
  return;
}
//
// vertex_main
//
Texture2DMS<float4> arg_0 : register(t0, space1);

float textureLoad_fcd23d() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = 1;
  float res = arg_0.Load(uint3(arg_1, uint(0)), arg_2).x;
  return res;
}

struct VertexOutput {
  float4 pos;
  float prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation float prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureLoad_fcd23d();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
