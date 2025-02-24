struct frexp_result_f32 {
  float fract;
  int exp;
};


[numthreads(1, 1, 1)]
void main() {
  frexp_result_f32 res = {0.625f, int(1)};
  float fract = res.fract;
  int exp = res.exp;
}

