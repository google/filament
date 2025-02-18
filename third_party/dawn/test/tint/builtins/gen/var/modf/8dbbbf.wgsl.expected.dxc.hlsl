//
// fragment_main
//
struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};
modf_result_f16 tint_modf(float16_t param_0) {
  modf_result_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

void modf_8dbbbf() {
  float16_t arg_0 = float16_t(-1.5h);
  modf_result_f16 res = tint_modf(arg_0);
}

void fragment_main() {
  modf_8dbbbf();
  return;
}
//
// compute_main
//
struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};
modf_result_f16 tint_modf(float16_t param_0) {
  modf_result_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

void modf_8dbbbf() {
  float16_t arg_0 = float16_t(-1.5h);
  modf_result_f16 res = tint_modf(arg_0);
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_8dbbbf();
  return;
}
//
// vertex_main
//
struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};
modf_result_f16 tint_modf(float16_t param_0) {
  modf_result_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

void modf_8dbbbf() {
  float16_t arg_0 = float16_t(-1.5h);
  modf_result_f16 res = tint_modf(arg_0);
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
  modf_8dbbbf();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
