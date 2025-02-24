//
// fragment_main
//
struct frexp_result_vec4_f32 {
  float4 fract;
  int4 exp;
};
frexp_result_vec4_f32 tint_frexp(float4 param_0) {
  float4 exp;
  float4 fract = sign(param_0) * frexp(param_0, exp);
  frexp_result_vec4_f32 result = {fract, int4(exp)};
  return result;
}

void frexp_77af93() {
  float4 arg_0 = (1.0f).xxxx;
  frexp_result_vec4_f32 res = tint_frexp(arg_0);
}

void fragment_main() {
  frexp_77af93();
  return;
}
//
// compute_main
//
struct frexp_result_vec4_f32 {
  float4 fract;
  int4 exp;
};
frexp_result_vec4_f32 tint_frexp(float4 param_0) {
  float4 exp;
  float4 fract = sign(param_0) * frexp(param_0, exp);
  frexp_result_vec4_f32 result = {fract, int4(exp)};
  return result;
}

void frexp_77af93() {
  float4 arg_0 = (1.0f).xxxx;
  frexp_result_vec4_f32 res = tint_frexp(arg_0);
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_77af93();
  return;
}
//
// vertex_main
//
struct frexp_result_vec4_f32 {
  float4 fract;
  int4 exp;
};
frexp_result_vec4_f32 tint_frexp(float4 param_0) {
  float4 exp;
  float4 fract = sign(param_0) * frexp(param_0, exp);
  frexp_result_vec4_f32 result = {fract, int4(exp)};
  return result;
}

void frexp_77af93() {
  float4 arg_0 = (1.0f).xxxx;
  frexp_result_vec4_f32 res = tint_frexp(arg_0);
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
  frexp_77af93();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
