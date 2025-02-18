[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float t = 0.0f;

float4 m() {
  t = 1.0f;
  return float4((t).xxxx);
}

void f() {
  float4 tint_symbol = m();
  bool4 v = bool4(tint_symbol);
}
