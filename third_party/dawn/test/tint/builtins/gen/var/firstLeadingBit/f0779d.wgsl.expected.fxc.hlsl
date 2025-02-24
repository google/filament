//
// fragment_main
//
uint tint_first_leading_bit(uint v) {
  uint x = v;
  uint b16 = (bool((x & 4294901760u)) ? 16u : 0u);
  x = (x >> b16);
  uint b8 = (bool((x & 65280u)) ? 8u : 0u);
  x = (x >> b8);
  uint b4 = (bool((x & 240u)) ? 4u : 0u);
  x = (x >> b4);
  uint b2 = (bool((x & 12u)) ? 2u : 0u);
  x = (x >> b2);
  uint b1 = (bool((x & 2u)) ? 1u : 0u);
  uint is_zero = ((x == 0u) ? 4294967295u : 0u);
  return uint((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint res = tint_first_leading_bit(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(firstLeadingBit_f0779d()));
  return;
}
//
// compute_main
//
uint tint_first_leading_bit(uint v) {
  uint x = v;
  uint b16 = (bool((x & 4294901760u)) ? 16u : 0u);
  x = (x >> b16);
  uint b8 = (bool((x & 65280u)) ? 8u : 0u);
  x = (x >> b8);
  uint b4 = (bool((x & 240u)) ? 4u : 0u);
  x = (x >> b4);
  uint b2 = (bool((x & 12u)) ? 2u : 0u);
  x = (x >> b2);
  uint b1 = (bool((x & 2u)) ? 1u : 0u);
  uint is_zero = ((x == 0u) ? 4294967295u : 0u);
  return uint((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint res = tint_first_leading_bit(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(firstLeadingBit_f0779d()));
  return;
}
//
// vertex_main
//
uint tint_first_leading_bit(uint v) {
  uint x = v;
  uint b16 = (bool((x & 4294901760u)) ? 16u : 0u);
  x = (x >> b16);
  uint b8 = (bool((x & 65280u)) ? 8u : 0u);
  x = (x >> b8);
  uint b4 = (bool((x & 240u)) ? 4u : 0u);
  x = (x >> b4);
  uint b2 = (bool((x & 12u)) ? 2u : 0u);
  x = (x >> b2);
  uint b1 = (bool((x & 2u)) ? 1u : 0u);
  uint is_zero = ((x == 0u) ? 4294967295u : 0u);
  return uint((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint res = tint_first_leading_bit(arg_0);
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
  tint_symbol.prevent_dce = firstLeadingBit_f0779d();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
