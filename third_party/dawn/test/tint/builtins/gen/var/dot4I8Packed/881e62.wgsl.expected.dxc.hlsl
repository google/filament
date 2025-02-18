//
// fragment_main
//
int tint_dot4I8Packed(uint param_0, uint param_1) {
  int accumulator = 0;
  return dot4add_i8packed(param_0, param_1, accumulator);
}

RWByteAddressBuffer prevent_dce : register(u0);

int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  int res = tint_dot4I8Packed(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(dot4I8Packed_881e62()));
  return;
}
//
// compute_main
//
int tint_dot4I8Packed(uint param_0, uint param_1) {
  int accumulator = 0;
  return dot4add_i8packed(param_0, param_1, accumulator);
}

RWByteAddressBuffer prevent_dce : register(u0);

int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  int res = tint_dot4I8Packed(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(dot4I8Packed_881e62()));
  return;
}
//
// vertex_main
//
int tint_dot4I8Packed(uint param_0, uint param_1) {
  int accumulator = 0;
  return dot4add_i8packed(param_0, param_1, accumulator);
}

int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  int res = tint_dot4I8Packed(arg_0, arg_1);
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
  tint_symbol.prevent_dce = dot4I8Packed_881e62();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
