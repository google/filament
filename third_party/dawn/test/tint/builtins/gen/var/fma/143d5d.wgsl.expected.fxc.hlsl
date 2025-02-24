//
// fragment_main
//
void fma_143d5d() {
  float4 res = (2.0f).xxxx;
}

void fragment_main() {
  fma_143d5d();
  return;
}
//
// compute_main
//
void fma_143d5d() {
  float4 res = (2.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fma_143d5d();
  return;
}
//
// vertex_main
//
void fma_143d5d() {
  float4 res = (2.0f).xxxx;
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
  fma_143d5d();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
