SKIP: INVALID

struct frexp_result_vec3_f16 {
  vector<float16_t, 3> fract;
  int3 exp;
};
frexp_result_vec3_f16 tint_frexp(vector<float16_t, 3> param_0) {
  vector<float16_t, 3> exp;
  vector<float16_t, 3> fract = sign(param_0) * frexp(param_0, exp);
  frexp_result_vec3_f16 result = {fract, int3(exp)};
  return result;
}

void frexp_ae4a66() {
  vector<float16_t, 3> arg_0 = (float16_t(1.0h)).xxx;
  frexp_result_vec3_f16 res = tint_frexp(arg_0);
}

void fragment_main() {
  frexp_ae4a66();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_ae4a66();
  return;
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
  frexp_ae4a66();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
FXC validation failure:
<scrubbed_path>(2,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
