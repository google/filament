//
// fragment_main
//
uint4 tint_count_leading_zeros(uint4 v) {
  uint4 x = uint4(v);
  uint4 b16 = ((x <= (65535u).xxxx) ? (16u).xxxx : (0u).xxxx);
  x = (x << b16);
  uint4 b8 = ((x <= (16777215u).xxxx) ? (8u).xxxx : (0u).xxxx);
  x = (x << b8);
  uint4 b4 = ((x <= (268435455u).xxxx) ? (4u).xxxx : (0u).xxxx);
  x = (x << b4);
  uint4 b2 = ((x <= (1073741823u).xxxx) ? (2u).xxxx : (0u).xxxx);
  x = (x << b2);
  uint4 b1 = ((x <= (2147483647u).xxxx) ? (1u).xxxx : (0u).xxxx);
  uint4 is_zero = ((x == (0u).xxxx) ? (1u).xxxx : (0u).xxxx);
  return uint4((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint4 countLeadingZeros_f70103() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = tint_count_leading_zeros(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(countLeadingZeros_f70103()));
  return;
}
//
// compute_main
//
uint4 tint_count_leading_zeros(uint4 v) {
  uint4 x = uint4(v);
  uint4 b16 = ((x <= (65535u).xxxx) ? (16u).xxxx : (0u).xxxx);
  x = (x << b16);
  uint4 b8 = ((x <= (16777215u).xxxx) ? (8u).xxxx : (0u).xxxx);
  x = (x << b8);
  uint4 b4 = ((x <= (268435455u).xxxx) ? (4u).xxxx : (0u).xxxx);
  x = (x << b4);
  uint4 b2 = ((x <= (1073741823u).xxxx) ? (2u).xxxx : (0u).xxxx);
  x = (x << b2);
  uint4 b1 = ((x <= (2147483647u).xxxx) ? (1u).xxxx : (0u).xxxx);
  uint4 is_zero = ((x == (0u).xxxx) ? (1u).xxxx : (0u).xxxx);
  return uint4((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint4 countLeadingZeros_f70103() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = tint_count_leading_zeros(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(countLeadingZeros_f70103()));
  return;
}
//
// vertex_main
//
uint4 tint_count_leading_zeros(uint4 v) {
  uint4 x = uint4(v);
  uint4 b16 = ((x <= (65535u).xxxx) ? (16u).xxxx : (0u).xxxx);
  x = (x << b16);
  uint4 b8 = ((x <= (16777215u).xxxx) ? (8u).xxxx : (0u).xxxx);
  x = (x << b8);
  uint4 b4 = ((x <= (268435455u).xxxx) ? (4u).xxxx : (0u).xxxx);
  x = (x << b4);
  uint4 b2 = ((x <= (1073741823u).xxxx) ? (2u).xxxx : (0u).xxxx);
  x = (x << b2);
  uint4 b1 = ((x <= (2147483647u).xxxx) ? (1u).xxxx : (0u).xxxx);
  uint4 is_zero = ((x == (0u).xxxx) ? (1u).xxxx : (0u).xxxx);
  return uint4((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

uint4 countLeadingZeros_f70103() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = tint_count_leading_zeros(arg_0);
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
  tint_symbol.prevent_dce = countLeadingZeros_f70103();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
