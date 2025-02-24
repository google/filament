struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};


[numthreads(1, 1, 1)]
void main() {
  float2 v = float2(1.25f, 3.75f);
  float2 v_1 = (0.0f).xx;
  float2 v_2 = frexp(v, v_1);
  float2 v_3 = (float2(sign(v)) * v_2);
  frexp_result_vec2_f32 res = {v_3, int2(v_1)};
  float2 fract = res.fract;
  int2 exp = res.exp;
}

