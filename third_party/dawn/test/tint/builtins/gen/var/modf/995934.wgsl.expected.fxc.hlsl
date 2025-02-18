SKIP: INVALID

struct modf_result_vec4_f16 {
  vector<float16_t, 4> fract;
  vector<float16_t, 4> whole;
};
modf_result_vec4_f16 tint_modf(vector<float16_t, 4> param_0) {
  modf_result_vec4_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

void modf_995934() {
  vector<float16_t, 4> arg_0 = (float16_t(-1.5h)).xxxx;
  modf_result_vec4_f16 res = tint_modf(arg_0);
}

void fragment_main() {
  modf_995934();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_995934();
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
  modf_995934();
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
