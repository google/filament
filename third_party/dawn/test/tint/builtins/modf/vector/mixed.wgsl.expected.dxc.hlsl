struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};
modf_result_vec2_f32 tint_modf(float2 param_0) {
  modf_result_vec2_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

[numthreads(1, 1, 1)]
void main() {
  float2 runtime_in = float2(1.25f, 3.75f);
  modf_result_vec2_f32 res = {float2(0.25f, 0.75f), float2(1.0f, 3.0f)};
  res = tint_modf(runtime_in);
  modf_result_vec2_f32 tint_symbol = {float2(0.25f, 0.75f), float2(1.0f, 3.0f)};
  res = tint_symbol;
  float2 fract = res.fract;
  float2 whole = res.whole;
  return;
}
