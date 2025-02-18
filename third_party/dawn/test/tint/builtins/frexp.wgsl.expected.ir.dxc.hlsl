struct frexp_result_f32 {
  float fract;
  int exp;
};


[numthreads(1, 1, 1)]
void main() {
  frexp_result_f32 res = {0.61500000953674316406f, int(1)};
  int exp = res.exp;
  float fract = res.fract;
}

