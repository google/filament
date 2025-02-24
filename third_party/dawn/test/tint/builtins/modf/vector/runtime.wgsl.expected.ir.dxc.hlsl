struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};


[numthreads(1, 1, 1)]
void main() {
  float2 v = float2(1.25f, 3.75f);
  float2 v_1 = (0.0f).xx;
  modf_result_vec2_f32 res = {modf(v, v_1), v_1};
  float2 fract = res.fract;
  float2 whole = res.whole;
}

