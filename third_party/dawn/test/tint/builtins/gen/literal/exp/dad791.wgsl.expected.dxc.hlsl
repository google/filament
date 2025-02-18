//
// fragment_main
//
void exp_dad791() {
  float4 res = (2.71828174591064453125f).xxxx;
}

void fragment_main() {
  exp_dad791();
  return;
}
//
// compute_main
//
void exp_dad791() {
  float4 res = (2.71828174591064453125f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp_dad791();
  return;
}
//
// vertex_main
//
void exp_dad791() {
  float4 res = (2.71828174591064453125f).xxxx;
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
  exp_dad791();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
