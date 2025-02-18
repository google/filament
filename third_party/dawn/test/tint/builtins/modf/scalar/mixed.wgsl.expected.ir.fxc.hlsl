struct modf_result_f32 {
  float fract;
  float whole;
};


[numthreads(1, 1, 1)]
void main() {
  float runtime_in = 1.25f;
  modf_result_f32 res = {0.25f, 1.0f};
  float v = 0.0f;
  modf_result_f32 v_1 = {modf(runtime_in, v), v};
  res = v_1;
  modf_result_f32 v_2 = {0.25f, 1.0f};
  res = v_2;
  float fract = res.fract;
  float whole = res.whole;
}

