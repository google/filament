[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float t = 0.0f;

float2 m() {
  t = 1.0f;
  return float2((t).xx);
}

void f() {
  float2 tint_symbol = m();
  bool2 v = bool2(tint_symbol);
}
