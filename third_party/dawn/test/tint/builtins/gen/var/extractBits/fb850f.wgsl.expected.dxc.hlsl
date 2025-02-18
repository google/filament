//
// fragment_main
//
int4 tint_extract_bits(int4 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  int4 shl_result = ((shl < 32u) ? (v << uint4((shl).xxxx)) : (0).xxxx);
  return ((shr < 32u) ? (shl_result >> uint4((shr).xxxx)) : ((shl_result >> (31u).xxxx) >> (1u).xxxx));
}

RWByteAddressBuffer prevent_dce : register(u0);

int4 extractBits_fb850f() {
  int4 arg_0 = (1).xxxx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int4 res = tint_extract_bits(arg_0, arg_1, arg_2);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(extractBits_fb850f()));
  return;
}
//
// compute_main
//
int4 tint_extract_bits(int4 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  int4 shl_result = ((shl < 32u) ? (v << uint4((shl).xxxx)) : (0).xxxx);
  return ((shr < 32u) ? (shl_result >> uint4((shr).xxxx)) : ((shl_result >> (31u).xxxx) >> (1u).xxxx));
}

RWByteAddressBuffer prevent_dce : register(u0);

int4 extractBits_fb850f() {
  int4 arg_0 = (1).xxxx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int4 res = tint_extract_bits(arg_0, arg_1, arg_2);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(extractBits_fb850f()));
  return;
}
//
// vertex_main
//
int4 tint_extract_bits(int4 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  int4 shl_result = ((shl < 32u) ? (v << uint4((shl).xxxx)) : (0).xxxx);
  return ((shr < 32u) ? (shl_result >> uint4((shr).xxxx)) : ((shl_result >> (31u).xxxx) >> (1u).xxxx));
}

int4 extractBits_fb850f() {
  int4 arg_0 = (1).xxxx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int4 res = tint_extract_bits(arg_0, arg_1, arg_2);
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
  tint_symbol.prevent_dce = extractBits_fb850f();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
