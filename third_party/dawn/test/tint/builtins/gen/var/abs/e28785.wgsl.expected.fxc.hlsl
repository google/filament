//
// fragment_main
//
void abs_e28785() {
  float4 res = (1.0f).xxxx;
}

void fragment_main() {
  abs_e28785();
  return;
}
//
// compute_main
//
void abs_e28785() {
  float4 res = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_e28785();
  return;
}
//
// vertex_main
//
void abs_e28785() {
  float4 res = (1.0f).xxxx;
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
  abs_e28785();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
