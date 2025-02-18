//
// fragment_main
//
int3 tint_clamp(int3 e, int3 low, int3 high) {
  return min(max(e, low), high);
}

RWByteAddressBuffer prevent_dce : register(u0);

int3 clamp_5f0819() {
  int3 arg_0 = (1).xxx;
  int3 arg_1 = (1).xxx;
  int3 arg_2 = (1).xxx;
  int3 res = tint_clamp(arg_0, arg_1, arg_2);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(clamp_5f0819()));
  return;
}
//
// compute_main
//
int3 tint_clamp(int3 e, int3 low, int3 high) {
  return min(max(e, low), high);
}

RWByteAddressBuffer prevent_dce : register(u0);

int3 clamp_5f0819() {
  int3 arg_0 = (1).xxx;
  int3 arg_1 = (1).xxx;
  int3 arg_2 = (1).xxx;
  int3 res = tint_clamp(arg_0, arg_1, arg_2);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(clamp_5f0819()));
  return;
}
//
// vertex_main
//
int3 tint_clamp(int3 e, int3 low, int3 high) {
  return min(max(e, low), high);
}

int3 clamp_5f0819() {
  int3 arg_0 = (1).xxx;
  int3 arg_1 = (1).xxx;
  int3 arg_2 = (1).xxx;
  int3 res = tint_clamp(arg_0, arg_1, arg_2);
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
  tint_symbol.prevent_dce = clamp_5f0819();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
