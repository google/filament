[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float t = 0.0f;

float2x2 m() {
  t = (t + 1.0f);
  return float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
}

void f() {
  float2x2 tint_symbol = m();
  matrix<float16_t, 2, 2> v = matrix<float16_t, 2, 2>(tint_symbol);
}
