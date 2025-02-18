//
// fragment_main
//
uint3 tint_count_leading_zeros(uint3 v) {
  uint3 x = uint3(v);
  uint3 b16 = ((x <= (65535u).xxx) ? (16u).xxx : (0u).xxx);
  x = (x << b16);
  uint3 b8 = ((x <= (16777215u).xxx) ? (8u).xxx : (0u).xxx);
  x = (x << b8);
  uint3 b4 = ((x <= (268435455u).xxx) ? (4u).xxx : (0u).xxx);
  x = (x << b4);
  uint3 b2 = ((x <= (1073741823u).xxx) ? (2u).xxx : (0u).xxx);
  x = (x << b2);
  uint3 b1 = ((x <= (2147483647u).xxx) ? (1u).xxx : (0u).xxx);
  uint3 is_zero = ((x == (0u).xxx) ? (1u).xxx : (0u).xxx);
  return uint3((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint3 countLeadingZeros_ab6345() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = tint_count_leading_zeros(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(countLeadingZeros_ab6345()));
  return;
}
//
// compute_main
//
uint3 tint_count_leading_zeros(uint3 v) {
  uint3 x = uint3(v);
  uint3 b16 = ((x <= (65535u).xxx) ? (16u).xxx : (0u).xxx);
  x = (x << b16);
  uint3 b8 = ((x <= (16777215u).xxx) ? (8u).xxx : (0u).xxx);
  x = (x << b8);
  uint3 b4 = ((x <= (268435455u).xxx) ? (4u).xxx : (0u).xxx);
  x = (x << b4);
  uint3 b2 = ((x <= (1073741823u).xxx) ? (2u).xxx : (0u).xxx);
  x = (x << b2);
  uint3 b1 = ((x <= (2147483647u).xxx) ? (1u).xxx : (0u).xxx);
  uint3 is_zero = ((x == (0u).xxx) ? (1u).xxx : (0u).xxx);
  return uint3((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint3 countLeadingZeros_ab6345() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = tint_count_leading_zeros(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(countLeadingZeros_ab6345()));
  return;
}
//
// vertex_main
//
uint3 tint_count_leading_zeros(uint3 v) {
  uint3 x = uint3(v);
  uint3 b16 = ((x <= (65535u).xxx) ? (16u).xxx : (0u).xxx);
  x = (x << b16);
  uint3 b8 = ((x <= (16777215u).xxx) ? (8u).xxx : (0u).xxx);
  x = (x << b8);
  uint3 b4 = ((x <= (268435455u).xxx) ? (4u).xxx : (0u).xxx);
  x = (x << b4);
  uint3 b2 = ((x <= (1073741823u).xxx) ? (2u).xxx : (0u).xxx);
  x = (x << b2);
  uint3 b1 = ((x <= (2147483647u).xxx) ? (1u).xxx : (0u).xxx);
  uint3 is_zero = ((x == (0u).xxx) ? (1u).xxx : (0u).xxx);
  return uint3((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

uint3 countLeadingZeros_ab6345() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = tint_count_leading_zeros(arg_0);
  return res;
}

struct VertexOutput {
  float4 pos;
  uint3 prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation uint3 prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = countLeadingZeros_ab6345();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
