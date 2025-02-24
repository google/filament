[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

uint4 tint_ftou(float4 v) {
  return ((v <= (4294967040.0f).xxxx) ? ((v < (0.0f).xxxx) ? (0u).xxxx : uint4(v)) : (4294967295u).xxxx);
}

static float t = 0.0f;

float4 m() {
  t = 1.0f;
  return float4((t).xxxx);
}

void f() {
  float4 tint_symbol = m();
  uint4 v = tint_ftou(tint_symbol);
}
