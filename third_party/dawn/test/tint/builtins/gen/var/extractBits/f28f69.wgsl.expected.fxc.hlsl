//
// fragment_main
//
uint2 tint_extract_bits(uint2 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  uint2 shl_result = ((shl < 32u) ? (v << uint2((shl).xx)) : (0u).xx);
  return ((shr < 32u) ? (shl_result >> uint2((shr).xx)) : ((shl_result >> (31u).xx) >> (1u).xx));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint2 extractBits_f28f69() {
  uint2 arg_0 = (1u).xx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint2 res = tint_extract_bits(arg_0, arg_1, arg_2);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(extractBits_f28f69()));
  return;
}
//
// compute_main
//
uint2 tint_extract_bits(uint2 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  uint2 shl_result = ((shl < 32u) ? (v << uint2((shl).xx)) : (0u).xx);
  return ((shr < 32u) ? (shl_result >> uint2((shr).xx)) : ((shl_result >> (31u).xx) >> (1u).xx));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint2 extractBits_f28f69() {
  uint2 arg_0 = (1u).xx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint2 res = tint_extract_bits(arg_0, arg_1, arg_2);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(extractBits_f28f69()));
  return;
}
//
// vertex_main
//
uint2 tint_extract_bits(uint2 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  uint2 shl_result = ((shl < 32u) ? (v << uint2((shl).xx)) : (0u).xx);
  return ((shr < 32u) ? (shl_result >> uint2((shr).xx)) : ((shl_result >> (31u).xx) >> (1u).xx));
}

uint2 extractBits_f28f69() {
  uint2 arg_0 = (1u).xx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint2 res = tint_extract_bits(arg_0, arg_1, arg_2);
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
  tint_symbol.prevent_dce = extractBits_f28f69();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
