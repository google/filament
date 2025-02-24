//
// fragment_main
//
int4 tint_first_leading_bit(int4 v) {
  uint4 x = ((v < (0).xxxx) ? uint4(~(v)) : uint4(v));
  uint4 b16 = (bool4((x & (4294901760u).xxxx)) ? (16u).xxxx : (0u).xxxx);
  x = (x >> b16);
  uint4 b8 = (bool4((x & (65280u).xxxx)) ? (8u).xxxx : (0u).xxxx);
  x = (x >> b8);
  uint4 b4 = (bool4((x & (240u).xxxx)) ? (4u).xxxx : (0u).xxxx);
  x = (x >> b4);
  uint4 b2 = (bool4((x & (12u).xxxx)) ? (2u).xxxx : (0u).xxxx);
  x = (x >> b2);
  uint4 b1 = (bool4((x & (2u).xxxx)) ? (1u).xxxx : (0u).xxxx);
  uint4 is_zero = ((x == (0u).xxxx) ? (4294967295u).xxxx : (0u).xxxx);
  return int4((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

int4 firstLeadingBit_c1f940() {
  int4 arg_0 = (1).xxxx;
  int4 res = tint_first_leading_bit(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(firstLeadingBit_c1f940()));
  return;
}
//
// compute_main
//
int4 tint_first_leading_bit(int4 v) {
  uint4 x = ((v < (0).xxxx) ? uint4(~(v)) : uint4(v));
  uint4 b16 = (bool4((x & (4294901760u).xxxx)) ? (16u).xxxx : (0u).xxxx);
  x = (x >> b16);
  uint4 b8 = (bool4((x & (65280u).xxxx)) ? (8u).xxxx : (0u).xxxx);
  x = (x >> b8);
  uint4 b4 = (bool4((x & (240u).xxxx)) ? (4u).xxxx : (0u).xxxx);
  x = (x >> b4);
  uint4 b2 = (bool4((x & (12u).xxxx)) ? (2u).xxxx : (0u).xxxx);
  x = (x >> b2);
  uint4 b1 = (bool4((x & (2u).xxxx)) ? (1u).xxxx : (0u).xxxx);
  uint4 is_zero = ((x == (0u).xxxx) ? (4294967295u).xxxx : (0u).xxxx);
  return int4((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

int4 firstLeadingBit_c1f940() {
  int4 arg_0 = (1).xxxx;
  int4 res = tint_first_leading_bit(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(firstLeadingBit_c1f940()));
  return;
}
//
// vertex_main
//
int4 tint_first_leading_bit(int4 v) {
  uint4 x = ((v < (0).xxxx) ? uint4(~(v)) : uint4(v));
  uint4 b16 = (bool4((x & (4294901760u).xxxx)) ? (16u).xxxx : (0u).xxxx);
  x = (x >> b16);
  uint4 b8 = (bool4((x & (65280u).xxxx)) ? (8u).xxxx : (0u).xxxx);
  x = (x >> b8);
  uint4 b4 = (bool4((x & (240u).xxxx)) ? (4u).xxxx : (0u).xxxx);
  x = (x >> b4);
  uint4 b2 = (bool4((x & (12u).xxxx)) ? (2u).xxxx : (0u).xxxx);
  x = (x >> b2);
  uint4 b1 = (bool4((x & (2u).xxxx)) ? (1u).xxxx : (0u).xxxx);
  uint4 is_zero = ((x == (0u).xxxx) ? (4294967295u).xxxx : (0u).xxxx);
  return int4((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

int4 firstLeadingBit_c1f940() {
  int4 arg_0 = (1).xxxx;
  int4 res = tint_first_leading_bit(arg_0);
  return res;
}

struct VertexOutput {
  float4 pos;
  int4 prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation int4 prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = firstLeadingBit_c1f940();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
