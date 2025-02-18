//
// fragment_main
//
struct frexp_result_vec4_f16 {
  vector<float16_t, 4> fract;
  int4 exp;
};
void frexp_3dd21e() {
  frexp_result_vec4_f16 res = {(float16_t(0.5h)).xxxx, (1).xxxx};
}

void fragment_main() {
  frexp_3dd21e();
  return;
}
//
// compute_main
//
struct frexp_result_vec4_f16 {
  vector<float16_t, 4> fract;
  int4 exp;
};
void frexp_3dd21e() {
  frexp_result_vec4_f16 res = {(float16_t(0.5h)).xxxx, (1).xxxx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_3dd21e();
  return;
}
//
// vertex_main
//
struct frexp_result_vec4_f16 {
  vector<float16_t, 4> fract;
  int4 exp;
};
void frexp_3dd21e() {
  frexp_result_vec4_f16 res = {(float16_t(0.5h)).xxxx, (1).xxxx};
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
  frexp_3dd21e();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
