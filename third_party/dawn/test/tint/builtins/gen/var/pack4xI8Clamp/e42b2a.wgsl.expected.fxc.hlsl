//
// fragment_main
//
uint tint_pack_4xi8_clamp(int4 a) {
  int4 a_clamp = clamp(a, (-128).xxxx, (127).xxxx);
  uint4 a_u32 = asuint(a_clamp);
  uint4 a_u8 = ((a_u32 & (255u).xxxx) << uint4(0u, 8u, 16u, 24u));
  return dot(a_u8, (1u).xxxx);
}

RWByteAddressBuffer prevent_dce : register(u0);

uint pack4xI8Clamp_e42b2a() {
  int4 arg_0 = (1).xxxx;
  uint res = tint_pack_4xi8_clamp(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(pack4xI8Clamp_e42b2a()));
  return;
}
//
// compute_main
//
uint tint_pack_4xi8_clamp(int4 a) {
  int4 a_clamp = clamp(a, (-128).xxxx, (127).xxxx);
  uint4 a_u32 = asuint(a_clamp);
  uint4 a_u8 = ((a_u32 & (255u).xxxx) << uint4(0u, 8u, 16u, 24u));
  return dot(a_u8, (1u).xxxx);
}

RWByteAddressBuffer prevent_dce : register(u0);

uint pack4xI8Clamp_e42b2a() {
  int4 arg_0 = (1).xxxx;
  uint res = tint_pack_4xi8_clamp(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(pack4xI8Clamp_e42b2a()));
  return;
}
//
// vertex_main
//
uint tint_pack_4xi8_clamp(int4 a) {
  int4 a_clamp = clamp(a, (-128).xxxx, (127).xxxx);
  uint4 a_u32 = asuint(a_clamp);
  uint4 a_u8 = ((a_u32 & (255u).xxxx) << uint4(0u, 8u, 16u, 24u));
  return dot(a_u8, (1u).xxxx);
}

uint pack4xI8Clamp_e42b2a() {
  int4 arg_0 = (1).xxxx;
  uint res = tint_pack_4xi8_clamp(arg_0);
  return res;
}

struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation uint prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = pack4xI8Clamp_e42b2a();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
