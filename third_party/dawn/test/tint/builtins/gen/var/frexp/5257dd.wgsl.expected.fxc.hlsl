SKIP: INVALID

struct frexp_result_f16 {
  float16_t fract;
  int exp;
};
frexp_result_f16 tint_frexp(float16_t param_0) {
  float16_t exp;
  float16_t fract = sign(param_0) * frexp(param_0, exp);
  frexp_result_f16 result = {fract, int(exp)};
  return result;
}

void frexp_5257dd() {
  float16_t arg_0 = float16_t(1.0h);
  frexp_result_f16 res = tint_frexp(arg_0);
}

void fragment_main() {
  frexp_5257dd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_5257dd();
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
  frexp_5257dd();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
FXC validation failure:
<scrubbed_path>(2,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
