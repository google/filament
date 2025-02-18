//
// fragment_main
//
int2 tint_insert_bits(int2 v, int2 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint2((offset).xx)) : (0).xx) & int2((int(mask)).xx)) | (v & int2((int(~(mask))).xx)));
}

RWByteAddressBuffer prevent_dce : register(u0);

int2 insertBits_fe6ba6() {
  int2 arg_0 = (1).xx;
  int2 arg_1 = (1).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int2 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(insertBits_fe6ba6()));
  return;
}
//
// compute_main
//
int2 tint_insert_bits(int2 v, int2 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint2((offset).xx)) : (0).xx) & int2((int(mask)).xx)) | (v & int2((int(~(mask))).xx)));
}

RWByteAddressBuffer prevent_dce : register(u0);

int2 insertBits_fe6ba6() {
  int2 arg_0 = (1).xx;
  int2 arg_1 = (1).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int2 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(insertBits_fe6ba6()));
  return;
}
//
// vertex_main
//
int2 tint_insert_bits(int2 v, int2 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint2((offset).xx)) : (0).xx) & int2((int(mask)).xx)) | (v & int2((int(~(mask))).xx)));
}

int2 insertBits_fe6ba6() {
  int2 arg_0 = (1).xx;
  int2 arg_1 = (1).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int2 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
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
  tint_symbol.prevent_dce = insertBits_fe6ba6();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
