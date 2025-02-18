struct modf_result_f32 {
  float fract;
  float whole;
};


[numthreads(1, 1, 1)]
void main() {
  float v = 1.25f;
  float v_1 = 0.0f;
  modf_result_f32 res = {modf(v, v_1), v_1};
  float fract = res.fract;
  float whole = res.whole;
}

