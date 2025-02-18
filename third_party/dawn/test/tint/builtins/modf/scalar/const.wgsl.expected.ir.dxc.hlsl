struct modf_result_f32 {
  float fract;
  float whole;
};


[numthreads(1, 1, 1)]
void main() {
  modf_result_f32 res = {0.25f, 1.0f};
  float fract = res.fract;
  float whole = res.whole;
}

