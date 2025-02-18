//
// fragment_main
//
uint2 tint_count_leading_zeros(uint2 v) {
  uint2 x = uint2(v);
  uint2 b16 = ((x <= (65535u).xx) ? (16u).xx : (0u).xx);
  x = (x << b16);
  uint2 b8 = ((x <= (16777215u).xx) ? (8u).xx : (0u).xx);
  x = (x << b8);
  uint2 b4 = ((x <= (268435455u).xx) ? (4u).xx : (0u).xx);
  x = (x << b4);
  uint2 b2 = ((x <= (1073741823u).xx) ? (2u).xx : (0u).xx);
  x = (x << b2);
  uint2 b1 = ((x <= (2147483647u).xx) ? (1u).xx : (0u).xx);
  uint2 is_zero = ((x == (0u).xx) ? (1u).xx : (0u).xx);
  return uint2((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint2 countLeadingZeros_70783f() {
  uint2 arg_0 = (1u).xx;
  uint2 res = tint_count_leading_zeros(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(countLeadingZeros_70783f()));
  return;
}
//
// compute_main
//
uint2 tint_count_leading_zeros(uint2 v) {
  uint2 x = uint2(v);
  uint2 b16 = ((x <= (65535u).xx) ? (16u).xx : (0u).xx);
  x = (x << b16);
  uint2 b8 = ((x <= (16777215u).xx) ? (8u).xx : (0u).xx);
  x = (x << b8);
  uint2 b4 = ((x <= (268435455u).xx) ? (4u).xx : (0u).xx);
  x = (x << b4);
  uint2 b2 = ((x <= (1073741823u).xx) ? (2u).xx : (0u).xx);
  x = (x << b2);
  uint2 b1 = ((x <= (2147483647u).xx) ? (1u).xx : (0u).xx);
  uint2 is_zero = ((x == (0u).xx) ? (1u).xx : (0u).xx);
  return uint2((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint2 countLeadingZeros_70783f() {
  uint2 arg_0 = (1u).xx;
  uint2 res = tint_count_leading_zeros(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(countLeadingZeros_70783f()));
  return;
}
//
// vertex_main
//
uint2 tint_count_leading_zeros(uint2 v) {
  uint2 x = uint2(v);
  uint2 b16 = ((x <= (65535u).xx) ? (16u).xx : (0u).xx);
  x = (x << b16);
  uint2 b8 = ((x <= (16777215u).xx) ? (8u).xx : (0u).xx);
  x = (x << b8);
  uint2 b4 = ((x <= (268435455u).xx) ? (4u).xx : (0u).xx);
  x = (x << b4);
  uint2 b2 = ((x <= (1073741823u).xx) ? (2u).xx : (0u).xx);
  x = (x << b2);
  uint2 b1 = ((x <= (2147483647u).xx) ? (1u).xx : (0u).xx);
  uint2 is_zero = ((x == (0u).xx) ? (1u).xx : (0u).xx);
  return uint2((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

uint2 countLeadingZeros_70783f() {
  uint2 arg_0 = (1u).xx;
  uint2 res = tint_count_leading_zeros(arg_0);
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
  tint_symbol.prevent_dce = countLeadingZeros_70783f();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
