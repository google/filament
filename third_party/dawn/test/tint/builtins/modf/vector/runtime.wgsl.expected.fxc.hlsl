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
  float2 tint_symbol = float2(1.25f, 3.75f);
  modf_result_vec2_f32 res = tint_modf(tint_symbol);
  float2 fract = res.fract;
  float2 whole = res.whole;
  return;
}
