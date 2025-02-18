//
// fragment_main
//
int3 tint_count_trailing_zeros(int3 v) {
  uint3 x = uint3(v);
  uint3 b16 = (bool3((x & (65535u).xxx)) ? (0u).xxx : (16u).xxx);
  x = (x >> b16);
  uint3 b8 = (bool3((x & (255u).xxx)) ? (0u).xxx : (8u).xxx);
  x = (x >> b8);
  uint3 b4 = (bool3((x & (15u).xxx)) ? (0u).xxx : (4u).xxx);
  x = (x >> b4);
  uint3 b2 = (bool3((x & (3u).xxx)) ? (0u).xxx : (2u).xxx);
  x = (x >> b2);
  uint3 b1 = (bool3((x & (1u).xxx)) ? (0u).xxx : (1u).xxx);
  uint3 is_zero = ((x == (0u).xxx) ? (1u).xxx : (0u).xxx);
  return int3((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

int3 countTrailingZeros_acfacb() {
  int3 arg_0 = (1).xxx;
  int3 res = tint_count_trailing_zeros(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(countTrailingZeros_acfacb()));
  return;
}
//
// compute_main
//
int3 tint_count_trailing_zeros(int3 v) {
  uint3 x = uint3(v);
  uint3 b16 = (bool3((x & (65535u).xxx)) ? (0u).xxx : (16u).xxx);
  x = (x >> b16);
  uint3 b8 = (bool3((x & (255u).xxx)) ? (0u).xxx : (8u).xxx);
  x = (x >> b8);
  uint3 b4 = (bool3((x & (15u).xxx)) ? (0u).xxx : (4u).xxx);
  x = (x >> b4);
  uint3 b2 = (bool3((x & (3u).xxx)) ? (0u).xxx : (2u).xxx);
  x = (x >> b2);
  uint3 b1 = (bool3((x & (1u).xxx)) ? (0u).xxx : (1u).xxx);
  uint3 is_zero = ((x == (0u).xxx) ? (1u).xxx : (0u).xxx);
  return int3((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0);

int3 countTrailingZeros_acfacb() {
  int3 arg_0 = (1).xxx;
  int3 res = tint_count_trailing_zeros(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(countTrailingZeros_acfacb()));
  return;
}
//
// vertex_main
//
int3 tint_count_trailing_zeros(int3 v) {
  uint3 x = uint3(v);
  uint3 b16 = (bool3((x & (65535u).xxx)) ? (0u).xxx : (16u).xxx);
  x = (x >> b16);
  uint3 b8 = (bool3((x & (255u).xxx)) ? (0u).xxx : (8u).xxx);
  x = (x >> b8);
  uint3 b4 = (bool3((x & (15u).xxx)) ? (0u).xxx : (4u).xxx);
  x = (x >> b4);
  uint3 b2 = (bool3((x & (3u).xxx)) ? (0u).xxx : (2u).xxx);
  x = (x >> b2);
  uint3 b1 = (bool3((x & (1u).xxx)) ? (0u).xxx : (1u).xxx);
  uint3 is_zero = ((x == (0u).xxx) ? (1u).xxx : (0u).xxx);
  return int3((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

int3 countTrailingZeros_acfacb() {
  int3 arg_0 = (1).xxx;
  int3 res = tint_count_trailing_zeros(arg_0);
  return res;
}

struct VertexOutput {
  float4 pos;
  int3 prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation int3 prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = countTrailingZeros_acfacb();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
