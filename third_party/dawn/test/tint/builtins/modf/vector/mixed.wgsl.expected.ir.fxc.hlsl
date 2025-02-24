struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};


[numthreads(1, 1, 1)]
void main() {
  float2 runtime_in = float2(1.25f, 3.75f);
  modf_result_vec2_f32 res = {float2(0.25f, 0.75f), float2(1.0f, 3.0f)};
  float2 v = (0.0f).xx;
  modf_result_vec2_f32 v_1 = {modf(runtime_in, v), v};
  res = v_1;
  modf_result_vec2_f32 v_2 = {float2(0.25f, 0.75f), float2(1.0f, 3.0f)};
  res = v_2;
  float2 fract = res.fract;
  float2 whole = res.whole;
}

