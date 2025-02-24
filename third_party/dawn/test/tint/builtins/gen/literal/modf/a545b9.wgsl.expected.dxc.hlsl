//
// fragment_main
//
struct modf_result_vec2_f16 {
  vector<float16_t, 2> fract;
  vector<float16_t, 2> whole;
};
void modf_a545b9() {
  modf_result_vec2_f16 res = {(float16_t(-0.5h)).xx, (float16_t(-1.0h)).xx};
}

void fragment_main() {
  modf_a545b9();
  return;
}
//
// compute_main
//
struct modf_result_vec2_f16 {
  vector<float16_t, 2> fract;
  vector<float16_t, 2> whole;
};
void modf_a545b9() {
  modf_result_vec2_f16 res = {(float16_t(-0.5h)).xx, (float16_t(-1.0h)).xx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_a545b9();
  return;
}
//
// vertex_main
//
struct modf_result_vec2_f16 {
  vector<float16_t, 2> fract;
  vector<float16_t, 2> whole;
};
void modf_a545b9() {
  modf_result_vec2_f16 res = {(float16_t(-0.5h)).xx, (float16_t(-1.0h)).xx};
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
  modf_a545b9();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
