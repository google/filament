[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint t = 0u;

uint m() {
  t = 1u;
  return uint(t);
}

void f() {
  uint tint_symbol = m();
  float v = float(tint_symbol);
}
