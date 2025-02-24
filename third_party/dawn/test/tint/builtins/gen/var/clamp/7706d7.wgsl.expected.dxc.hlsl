//
// fragment_main
//
uint2 tint_clamp(uint2 e, uint2 low, uint2 high) {
  return min(max(e, low), high);
}

RWByteAddressBuffer prevent_dce : register(u0);

uint2 clamp_7706d7() {
  uint2 arg_0 = (1u).xx;
  uint2 arg_1 = (1u).xx;
  uint2 arg_2 = (1u).xx;
  uint2 res = tint_clamp(arg_0, arg_1, arg_2);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(clamp_7706d7()));
  return;
}
//
// compute_main
//
uint2 tint_clamp(uint2 e, uint2 low, uint2 high) {
  return min(max(e, low), high);
}

RWByteAddressBuffer prevent_dce : register(u0);

uint2 clamp_7706d7() {
  uint2 arg_0 = (1u).xx;
  uint2 arg_1 = (1u).xx;
  uint2 arg_2 = (1u).xx;
  uint2 res = tint_clamp(arg_0, arg_1, arg_2);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(clamp_7706d7()));
  return;
}
//
// vertex_main
//
uint2 tint_clamp(uint2 e, uint2 low, uint2 high) {
  return min(max(e, low), high);
}

uint2 clamp_7706d7() {
  uint2 arg_0 = (1u).xx;
  uint2 arg_1 = (1u).xx;
  uint2 arg_2 = (1u).xx;
  uint2 res = tint_clamp(arg_0, arg_1, arg_2);
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
  tint_symbol.prevent_dce = clamp_7706d7();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
