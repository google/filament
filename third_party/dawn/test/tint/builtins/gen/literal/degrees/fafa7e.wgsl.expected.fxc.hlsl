//
// fragment_main
//
void degrees_fafa7e() {
  float res = 57.295780181884765625f;
}

void fragment_main() {
  degrees_fafa7e();
  return;
}
//
// compute_main
//
void degrees_fafa7e() {
  float res = 57.295780181884765625f;
}

[numthreads(1, 1, 1)]
void compute_main() {
  degrees_fafa7e();
  return;
}
//
// vertex_main
//
void degrees_fafa7e() {
  float res = 57.295780181884765625f;
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
  degrees_fafa7e();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
