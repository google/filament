[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

uint tint_ftou(float v) {
  return ((v <= 4294967040.0f) ? ((v < 0.0f) ? 0u : uint(v)) : 4294967295u);
}

static float u = 1.0f;

void f() {
  uint v = tint_ftou(u);
}
