[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint t = 0u;

uint2 m() {
  t = 1u;
  return uint2((t).xx);
}

void f() {
  uint2 tint_symbol = m();
  bool2 v = bool2(tint_symbol);
}
