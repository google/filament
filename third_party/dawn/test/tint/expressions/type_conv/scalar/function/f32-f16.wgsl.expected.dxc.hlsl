[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float t = 0.0f;

float m() {
  t = 1.0f;
  return float(t);
}

void f() {
  float tint_symbol = m();
  float16_t v = float16_t(tint_symbol);
}
