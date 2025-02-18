//
// fragment_main
//
uint4 tint_insert_bits(uint4 v, uint4 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint4((offset).xxxx)) : (0u).xxxx) & uint4((mask).xxxx)) | (v & uint4((~(mask)).xxxx)));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint4 insertBits_51ede1() {
  uint4 arg_0 = (1u).xxxx;
  uint4 arg_1 = (1u).xxxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint4 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(insertBits_51ede1()));
  return;
}
//
// compute_main
//
uint4 tint_insert_bits(uint4 v, uint4 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint4((offset).xxxx)) : (0u).xxxx) & uint4((mask).xxxx)) | (v & uint4((~(mask)).xxxx)));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint4 insertBits_51ede1() {
  uint4 arg_0 = (1u).xxxx;
  uint4 arg_1 = (1u).xxxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint4 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(insertBits_51ede1()));
  return;
}
//
// vertex_main
//
uint4 tint_insert_bits(uint4 v, uint4 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint4((offset).xxxx)) : (0u).xxxx) & uint4((mask).xxxx)) | (v & uint4((~(mask)).xxxx)));
}

uint4 insertBits_51ede1() {
  uint4 arg_0 = (1u).xxxx;
  uint4 arg_1 = (1u).xxxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint4 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

struct VertexOutput {
  float4 pos;
  uint4 prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation uint4 prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = insertBits_51ede1();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
