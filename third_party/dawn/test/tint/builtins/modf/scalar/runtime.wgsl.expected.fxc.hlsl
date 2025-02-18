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
  float tint_symbol = 1.25f;
  modf_result_f32 res = tint_modf(tint_symbol);
  float fract = res.fract;
  float whole = res.whole;
  return;
}
