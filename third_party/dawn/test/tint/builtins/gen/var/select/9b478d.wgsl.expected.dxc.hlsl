//
// fragment_main
//
void select_9b478d() {
  bool arg_2 = true;
  int res = (arg_2 ? 1 : 1);
}

void fragment_main() {
  select_9b478d();
  return;
}
//
// compute_main
//
void select_9b478d() {
  bool arg_2 = true;
  int res = (arg_2 ? 1 : 1);
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_9b478d();
  return;
}
//
// vertex_main
//
void select_9b478d() {
  bool arg_2 = true;
  int res = (arg_2 ? 1 : 1);
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
  select_9b478d();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
