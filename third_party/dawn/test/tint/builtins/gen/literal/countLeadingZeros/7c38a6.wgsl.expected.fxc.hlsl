//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 countLeadingZeros_7c38a6() {
  int3 res = (31).xxx;
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(countLeadingZeros_7c38a6()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 countLeadingZeros_7c38a6() {
  int3 res = (31).xxx;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(countLeadingZeros_7c38a6()));
  return;
}
//
// vertex_main
//
int3 countLeadingZeros_7c38a6() {
  int3 res = (31).xxx;
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
  tint_symbol.prevent_dce = countLeadingZeros_7c38a6();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
