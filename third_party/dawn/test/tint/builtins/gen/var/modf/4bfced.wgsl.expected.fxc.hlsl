//
// fragment_main
//
struct modf_result_vec4_f32 {
  float4 fract;
  float4 whole;
};
modf_result_vec4_f32 tint_modf(float4 param_0) {
  modf_result_vec4_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

void modf_4bfced() {
  float4 arg_0 = (-1.5f).xxxx;
  modf_result_vec4_f32 res = tint_modf(arg_0);
}

void fragment_main() {
  modf_4bfced();
  return;
}
//
// compute_main
//
struct modf_result_vec4_f32 {
  float4 fract;
  float4 whole;
};
modf_result_vec4_f32 tint_modf(float4 param_0) {
  modf_result_vec4_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

void modf_4bfced() {
  float4 arg_0 = (-1.5f).xxxx;
  modf_result_vec4_f32 res = tint_modf(arg_0);
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_4bfced();
  return;
}
//
// vertex_main
//
struct modf_result_vec4_f32 {
  float4 fract;
  float4 whole;
};
modf_result_vec4_f32 tint_modf(float4 param_0) {
  modf_result_vec4_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

void modf_4bfced() {
  float4 arg_0 = (-1.5f).xxxx;
  modf_result_vec4_f32 res = tint_modf(arg_0);
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
  modf_4bfced();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
