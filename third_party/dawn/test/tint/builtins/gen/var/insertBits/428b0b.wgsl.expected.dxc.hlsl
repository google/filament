//
// fragment_main
//
int3 tint_insert_bits(int3 v, int3 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint3((offset).xxx)) : (0).xxx) & int3((int(mask)).xxx)) | (v & int3((int(~(mask))).xxx)));
}

RWByteAddressBuffer prevent_dce : register(u0);

int3 insertBits_428b0b() {
  int3 arg_0 = (1).xxx;
  int3 arg_1 = (1).xxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int3 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(insertBits_428b0b()));
  return;
}
//
// compute_main
//
int3 tint_insert_bits(int3 v, int3 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint3((offset).xxx)) : (0).xxx) & int3((int(mask)).xxx)) | (v & int3((int(~(mask))).xxx)));
}

RWByteAddressBuffer prevent_dce : register(u0);

int3 insertBits_428b0b() {
  int3 arg_0 = (1).xxx;
  int3 arg_1 = (1).xxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int3 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(insertBits_428b0b()));
  return;
}
//
// vertex_main
//
int3 tint_insert_bits(int3 v, int3 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint3((offset).xxx)) : (0).xxx) & int3((int(mask)).xxx)) | (v & int3((int(~(mask))).xxx)));
}

int3 insertBits_428b0b() {
  int3 arg_0 = (1).xxx;
  int3 arg_1 = (1).xxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int3 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
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
  tint_symbol.prevent_dce = insertBits_428b0b();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
