struct frexp_result_f32 {
  float fract;
  int exp;
};


[numthreads(1, 1, 1)]
void main() {
  float v = 1.25f;
  float v_1 = 0.0f;
  float v_2 = frexp(v, v_1);
  float v_3 = (float(sign(v)) * v_2);
  frexp_result_f32 res = {v_3, int(v_1)};
  float fract = res.fract;
  int exp = res.exp;
}

