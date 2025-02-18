//
// fragment_main
//
int tint_extract_bits(int v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  int shl_result = ((shl < 32u) ? (v << shl) : 0);
  return ((shr < 32u) ? (shl_result >> shr) : ((shl_result >> 31u) >> 1u));
}

RWByteAddressBuffer prevent_dce : register(u0);

int extractBits_249874() {
  int arg_0 = 1;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int res = tint_extract_bits(arg_0, arg_1, arg_2);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(extractBits_249874()));
  return;
}
//
// compute_main
//
int tint_extract_bits(int v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  int shl_result = ((shl < 32u) ? (v << shl) : 0);
  return ((shr < 32u) ? (shl_result >> shr) : ((shl_result >> 31u) >> 1u));
}

RWByteAddressBuffer prevent_dce : register(u0);

int extractBits_249874() {
  int arg_0 = 1;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int res = tint_extract_bits(arg_0, arg_1, arg_2);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(extractBits_249874()));
  return;
}
//
// vertex_main
//
int tint_extract_bits(int v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  int shl_result = ((shl < 32u) ? (v << shl) : 0);
  return ((shr < 32u) ? (shl_result >> shr) : ((shl_result >> 31u) >> 1u));
}

int extractBits_249874() {
  int arg_0 = 1;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int res = tint_extract_bits(arg_0, arg_1, arg_2);
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
  tint_symbol.prevent_dce = extractBits_249874();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
