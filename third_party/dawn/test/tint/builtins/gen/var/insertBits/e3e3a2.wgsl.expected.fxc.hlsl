//
// fragment_main
//
uint tint_insert_bits(uint v, uint n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << offset) : 0u) & mask) | (v & ~(mask)));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint insertBits_e3e3a2() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(insertBits_e3e3a2()));
  return;
}
//
// compute_main
//
uint tint_insert_bits(uint v, uint n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << offset) : 0u) & mask) | (v & ~(mask)));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint insertBits_e3e3a2() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(insertBits_e3e3a2()));
  return;
}
//
// vertex_main
//
uint tint_insert_bits(uint v, uint n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << offset) : 0u) & mask) | (v & ~(mask)));
}

uint insertBits_e3e3a2() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
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
  tint_symbol.prevent_dce = insertBits_e3e3a2();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
