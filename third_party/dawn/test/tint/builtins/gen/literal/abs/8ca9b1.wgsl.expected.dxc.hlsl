//
// fragment_main
//
void abs_8ca9b1() {
  int4 res = (1).xxxx;
}

void fragment_main() {
  abs_8ca9b1();
  return;
}
//
// compute_main
//
void abs_8ca9b1() {
  int4 res = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_8ca9b1();
  return;
}
//
// vertex_main
//
void abs_8ca9b1() {
  int4 res = (1).xxxx;
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
  abs_8ca9b1();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
