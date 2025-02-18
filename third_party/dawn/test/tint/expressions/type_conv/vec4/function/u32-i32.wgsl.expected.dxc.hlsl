[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint t = 0u;

uint4 m() {
  t = 1u;
  return uint4((t).xxxx);
}

void f() {
  uint4 tint_symbol = m();
  int4 v = int4(tint_symbol);
}
