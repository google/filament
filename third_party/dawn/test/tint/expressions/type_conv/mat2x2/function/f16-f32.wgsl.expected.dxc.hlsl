[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float16_t t = float16_t(0.0h);

matrix<float16_t, 2, 2> m() {
  t = (t + float16_t(1.0h));
  return matrix<float16_t, 2, 2>(vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h)), vector<float16_t, 2>(float16_t(3.0h), float16_t(4.0h)));
}

void f() {
  matrix<float16_t, 2, 2> tint_symbol = m();
  float2x2 v = float2x2(tint_symbol);
}
