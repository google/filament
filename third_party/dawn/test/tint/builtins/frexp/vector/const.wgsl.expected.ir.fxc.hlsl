struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};


[numthreads(1, 1, 1)]
void main() {
  frexp_result_vec2_f32 res = {float2(0.625f, 0.9375f), int2(int(1), int(2))};
  float2 fract = res.fract;
  int2 exp = res.exp;
}

