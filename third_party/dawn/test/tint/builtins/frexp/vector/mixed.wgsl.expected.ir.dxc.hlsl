struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};


[numthreads(1, 1, 1)]
void main() {
  float2 runtime_in = float2(1.25f, 3.75f);
  frexp_result_vec2_f32 res = {float2(0.625f, 0.9375f), int2(int(1), int(2))};
  float2 v = (0.0f).xx;
  float2 v_1 = frexp(runtime_in, v);
  float2 v_2 = (float2(sign(runtime_in)) * v_1);
  frexp_result_vec2_f32 v_3 = {v_2, int2(v)};
  res = v_3;
  frexp_result_vec2_f32 v_4 = {float2(0.625f, 0.9375f), int2(int(1), int(2))};
  res = v_4;
  float2 fract = res.fract;
  int2 exp = res.exp;
}

