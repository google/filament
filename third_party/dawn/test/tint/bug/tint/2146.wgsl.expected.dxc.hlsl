void set_vector_element(inout vector<float16_t, 4> vec, int idx, float16_t val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

[numthreads(1, 1, 1)]
void main() {
  vector<float16_t, 4> a = (float16_t(0.0h)).xxxx;
  float16_t b = float16_t(1.0h);
  int tint_symbol_1 = 0;
  set_vector_element(a, min(uint(tint_symbol_1), 3u), (a[min(uint(tint_symbol_1), 3u)] + b));
  return;
}
