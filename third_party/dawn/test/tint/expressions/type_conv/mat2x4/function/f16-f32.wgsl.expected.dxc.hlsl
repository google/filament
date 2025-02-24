[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float16_t t = float16_t(0.0h);

matrix<float16_t, 2, 4> m() {
  t = (t + float16_t(1.0h));
  return matrix<float16_t, 2, 4>(vector<float16_t, 4>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h), float16_t(4.0h)), vector<float16_t, 4>(float16_t(5.0h), float16_t(6.0h), float16_t(7.0h), float16_t(8.0h)));
}

void f() {
  matrix<float16_t, 2, 4> tint_symbol = m();
  float2x4 v = float2x4(tint_symbol);
}
