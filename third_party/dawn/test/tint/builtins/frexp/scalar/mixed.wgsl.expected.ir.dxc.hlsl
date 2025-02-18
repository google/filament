struct frexp_result_f32 {
  float fract;
  int exp;
};


[numthreads(1, 1, 1)]
void main() {
  float runtime_in = 1.25f;
  frexp_result_f32 res = {0.625f, int(1)};
  float v = 0.0f;
  float v_1 = frexp(runtime_in, v);
  float v_2 = (float(sign(runtime_in)) * v_1);
  frexp_result_f32 v_3 = {v_2, int(v)};
  res = v_3;
  frexp_result_f32 v_4 = {0.625f, int(1)};
  res = v_4;
  float fract = res.fract;
  int exp = res.exp;
}

