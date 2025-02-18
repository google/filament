//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 bitcast_a8ea1b() {
  int3 res = (1).xxx;
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(bitcast_a8ea1b()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 bitcast_a8ea1b() {
  int3 res = (1).xxx;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(bitcast_a8ea1b()));
  return;
}
//
// vertex_main
//
int3 bitcast_a8ea1b() {
  int3 res = (1).xxx;
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
  tint_symbol.prevent_dce = bitcast_a8ea1b();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
