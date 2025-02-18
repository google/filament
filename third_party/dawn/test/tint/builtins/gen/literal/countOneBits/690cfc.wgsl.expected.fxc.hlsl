//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 countOneBits_690cfc() {
  uint3 res = (1u).xxx;
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(countOneBits_690cfc()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 countOneBits_690cfc() {
  uint3 res = (1u).xxx;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(countOneBits_690cfc()));
  return;
}
//
// vertex_main
//
uint3 countOneBits_690cfc() {
  uint3 res = (1u).xxx;
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
  tint_symbol.prevent_dce = countOneBits_690cfc();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
