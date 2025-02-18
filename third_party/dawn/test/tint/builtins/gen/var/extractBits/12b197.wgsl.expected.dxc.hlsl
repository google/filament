//
// fragment_main
//
uint3 tint_extract_bits(uint3 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  uint3 shl_result = ((shl < 32u) ? (v << uint3((shl).xxx)) : (0u).xxx);
  return ((shr < 32u) ? (shl_result >> uint3((shr).xxx)) : ((shl_result >> (31u).xxx) >> (1u).xxx));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint3 extractBits_12b197() {
  uint3 arg_0 = (1u).xxx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint3 res = tint_extract_bits(arg_0, arg_1, arg_2);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(extractBits_12b197()));
  return;
}
//
// compute_main
//
uint3 tint_extract_bits(uint3 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  uint3 shl_result = ((shl < 32u) ? (v << uint3((shl).xxx)) : (0u).xxx);
  return ((shr < 32u) ? (shl_result >> uint3((shr).xxx)) : ((shl_result >> (31u).xxx) >> (1u).xxx));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint3 extractBits_12b197() {
  uint3 arg_0 = (1u).xxx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint3 res = tint_extract_bits(arg_0, arg_1, arg_2);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(extractBits_12b197()));
  return;
}
//
// vertex_main
//
uint3 tint_extract_bits(uint3 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  uint3 shl_result = ((shl < 32u) ? (v << uint3((shl).xxx)) : (0u).xxx);
  return ((shr < 32u) ? (shl_result >> uint3((shr).xxx)) : ((shl_result >> (31u).xxx) >> (1u).xxx));
}

uint3 extractBits_12b197() {
  uint3 arg_0 = (1u).xxx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint3 res = tint_extract_bits(arg_0, arg_1, arg_2);
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
  tint_symbol.prevent_dce = extractBits_12b197();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
