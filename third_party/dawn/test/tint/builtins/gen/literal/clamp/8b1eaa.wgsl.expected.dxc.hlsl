//
// fragment_main
//
void clamp_8b1eaa() {
  int3 res = (1).xxx;
}

void fragment_main() {
  clamp_8b1eaa();
  return;
}
//
// compute_main
//
void clamp_8b1eaa() {
  int3 res = (1).xxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_8b1eaa();
  return;
}
//
// vertex_main
//
void clamp_8b1eaa() {
  int3 res = (1).xxx;
}

struct VertexOutput {
  float4 pos;
};
struct tint_symbol_1 {
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  clamp_8b1eaa();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
