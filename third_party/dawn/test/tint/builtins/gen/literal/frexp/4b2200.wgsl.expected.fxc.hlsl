//
// fragment_main
//
struct frexp_result_f32 {
  float fract;
  int exp;
};
void frexp_4b2200() {
  frexp_result_f32 res = {0.5f, 1};
}

void fragment_main() {
  frexp_4b2200();
  return;
}
//
// compute_main
//
struct frexp_result_f32 {
  float fract;
  int exp;
};
void frexp_4b2200() {
  frexp_result_f32 res = {0.5f, 1};
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_4b2200();
  return;
}
//
// vertex_main
//
struct frexp_result_f32 {
  float fract;
  int exp;
};
void frexp_4b2200() {
  frexp_result_f32 res = {0.5f, 1};
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
  frexp_4b2200();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
