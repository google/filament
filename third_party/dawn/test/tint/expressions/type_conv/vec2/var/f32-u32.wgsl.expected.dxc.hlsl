[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

uint2 tint_ftou(float2 v) {
  return ((v <= (4294967040.0f).xx) ? ((v < (0.0f).xx) ? (0u).xx : uint2(v)) : (4294967295u).xx);
}

static float2 u = (1.0f).xx;

void f() {
  uint2 v = tint_ftou(u);
}
