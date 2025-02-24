[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

uint2 tint_ftou(float2 v) {
  return ((v <= (4294967040.0f).xx) ? ((v < (0.0f).xx) ? (0u).xx : uint2(v)) : (4294967295u).xx);
}

static float t = 0.0f;

float2 m() {
  t = 1.0f;
  return float2((t).xx);
}

void f() {
  float2 tint_symbol = m();
  uint2 v = tint_ftou(tint_symbol);
}
