//
// fragment_main
//
uint2 tint_insert_bits(uint2 v, uint2 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint2((offset).xx)) : (0u).xx) & uint2((mask).xx)) | (v & uint2((~(mask)).xx)));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint2 insertBits_3c7ba5() {
  uint2 arg_0 = (1u).xx;
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint2 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(insertBits_3c7ba5()));
  return;
}
//
// compute_main
//
uint2 tint_insert_bits(uint2 v, uint2 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint2((offset).xx)) : (0u).xx) & uint2((mask).xx)) | (v & uint2((~(mask)).xx)));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint2 insertBits_3c7ba5() {
  uint2 arg_0 = (1u).xx;
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint2 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(insertBits_3c7ba5()));
  return;
}
//
// vertex_main
//
uint2 tint_insert_bits(uint2 v, uint2 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint2((offset).xx)) : (0u).xx) & uint2((mask).xx)) | (v & uint2((~(mask)).xx)));
}

uint2 insertBits_3c7ba5() {
  uint2 arg_0 = (1u).xx;
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint2 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

struct VertexOutput {
  float4 pos;
  uint2 prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation uint2 prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = insertBits_3c7ba5();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
