//
// fragment_main
//
struct modf_result_vec4_f16 {
  vector<float16_t, 4> fract;
  vector<float16_t, 4> whole;
};
void modf_995934() {
  modf_result_vec4_f16 res = {(float16_t(-0.5h)).xxxx, (float16_t(-1.0h)).xxxx};
}

void fragment_main() {
  modf_995934();
  return;
}
//
// compute_main
//
struct modf_result_vec4_f16 {
  vector<float16_t, 4> fract;
  vector<float16_t, 4> whole;
};
void modf_995934() {
  modf_result_vec4_f16 res = {(float16_t(-0.5h)).xxxx, (float16_t(-1.0h)).xxxx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_995934();
  return;
}
//
// vertex_main
//
struct modf_result_vec4_f16 {
  vector<float16_t, 4> fract;
  vector<float16_t, 4> whole;
};
void modf_995934() {
  modf_result_vec4_f16 res = {(float16_t(-0.5h)).xxxx, (float16_t(-1.0h)).xxxx};
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
  modf_995934();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
