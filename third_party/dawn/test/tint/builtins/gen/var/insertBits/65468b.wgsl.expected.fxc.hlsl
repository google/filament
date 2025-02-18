//
// fragment_main
//
int tint_insert_bits(int v, int n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << offset) : 0) & int(mask)) | (v & int(~(mask))));
}

RWByteAddressBuffer prevent_dce : register(u0);

int insertBits_65468b() {
  int arg_0 = 1;
  int arg_1 = 1;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(insertBits_65468b()));
  return;
}
//
// compute_main
//
int tint_insert_bits(int v, int n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << offset) : 0) & int(mask)) | (v & int(~(mask))));
}

RWByteAddressBuffer prevent_dce : register(u0);

int insertBits_65468b() {
  int arg_0 = 1;
  int arg_1 = 1;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(insertBits_65468b()));
  return;
}
//
// vertex_main
//
int tint_insert_bits(int v, int n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << offset) : 0) & int(mask)) | (v & int(~(mask))));
}

int insertBits_65468b() {
  int arg_0 = 1;
  int arg_1 = 1;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

struct VertexOutput {
  float4 pos;
  int prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation int prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = insertBits_65468b();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
