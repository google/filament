struct modf_result_f32 {
  float fract;
  float whole;
};
modf_result_f32 tint_modf(float param_0) {
  modf_result_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

[numthreads(1, 1, 1)]
void main() {
  float runtime_in = 1.25f;
  modf_result_f32 res = {0.25f, 1.0f};
  res = tint_modf(runtime_in);
  modf_result_f32 tint_symbol = {0.25f, 1.0f};
  res = tint_symbol;
  float fract = res.fract;
  float whole = res.whole;
  return;
}
