struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};
frexp_result_vec2_f32 tint_frexp(float2 param_0) {
  float2 exp;
  float2 fract = sign(param_0) * frexp(param_0, exp);
  frexp_result_vec2_f32 result = {fract, int2(exp)};
  return result;
}

[numthreads(1, 1, 1)]
void main() {
  float2 runtime_in = float2(1.25f, 3.75f);
  frexp_result_vec2_f32 res = {float2(0.625f, 0.9375f), int2(1, 2)};
  res = tint_frexp(runtime_in);
  frexp_result_vec2_f32 tint_symbol = {float2(0.625f, 0.9375f), int2(1, 2)};
  res = tint_symbol;
  float2 fract = res.fract;
  int2 exp = res.exp;
  return;
}
