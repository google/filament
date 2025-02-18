//
// fragment_main
//
int2 tint_first_leading_bit(int2 v) {
  uint2 x = ((v < (0).xx) ? uint2(~(v)) : uint2(v));
  uint2 b16 = (bool2((x & (4294901760u).xx)) ? (16u).xx : (0u).xx);
  x = (x >> b16);
  uint2 b8 = (bool2((x & (65280u).xx)) ? (8u).xx : (0u).xx);
  x = (x >> b8);
  uint2 b4 = (bool2((x & (240u).xx)) ? (4u).xx : (0u).xx);
  x = (x >> b4);
  uint2 b2 = (bool2((x & (12u).xx)) ? (2u).xx : (0u).xx);
  x = (x >> b2);
  uint2 b1 = (bool2((x & (2u).xx)) ? (1u).xx : (0u).xx);
  uint2 is_zero = ((x == (0u).xx) ? (4294967295u).xx : (0u).xx);
  return int2((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

int2 firstLeadingBit_a622c2() {
  int2 arg_0 = (1).xx;
  int2 res = tint_first_leading_bit(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(firstLeadingBit_a622c2()));
  return;
}
//
// compute_main
//
int2 tint_first_leading_bit(int2 v) {
  uint2 x = ((v < (0).xx) ? uint2(~(v)) : uint2(v));
  uint2 b16 = (bool2((x & (4294901760u).xx)) ? (16u).xx : (0u).xx);
  x = (x >> b16);
  uint2 b8 = (bool2((x & (65280u).xx)) ? (8u).xx : (0u).xx);
  x = (x >> b8);
  uint2 b4 = (bool2((x & (240u).xx)) ? (4u).xx : (0u).xx);
  x = (x >> b4);
  uint2 b2 = (bool2((x & (12u).xx)) ? (2u).xx : (0u).xx);
  x = (x >> b2);
  uint2 b1 = (bool2((x & (2u).xx)) ? (1u).xx : (0u).xx);
  uint2 is_zero = ((x == (0u).xx) ? (4294967295u).xx : (0u).xx);
  return int2((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

int2 firstLeadingBit_a622c2() {
  int2 arg_0 = (1).xx;
  int2 res = tint_first_leading_bit(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(firstLeadingBit_a622c2()));
  return;
}
//
// vertex_main
//
int2 tint_first_leading_bit(int2 v) {
  uint2 x = ((v < (0).xx) ? uint2(~(v)) : uint2(v));
  uint2 b16 = (bool2((x & (4294901760u).xx)) ? (16u).xx : (0u).xx);
  x = (x >> b16);
  uint2 b8 = (bool2((x & (65280u).xx)) ? (8u).xx : (0u).xx);
  x = (x >> b8);
  uint2 b4 = (bool2((x & (240u).xx)) ? (4u).xx : (0u).xx);
  x = (x >> b4);
  uint2 b2 = (bool2((x & (12u).xx)) ? (2u).xx : (0u).xx);
  x = (x >> b2);
  uint2 b1 = (bool2((x & (2u).xx)) ? (1u).xx : (0u).xx);
  uint2 is_zero = ((x == (0u).xx) ? (4294967295u).xx : (0u).xx);
  return int2((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

int2 firstLeadingBit_a622c2() {
  int2 arg_0 = (1).xx;
  int2 res = tint_first_leading_bit(arg_0);
  return res;
}

struct VertexOutput {
  float4 pos;
  int2 prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation int2 prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = firstLeadingBit_a622c2();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
