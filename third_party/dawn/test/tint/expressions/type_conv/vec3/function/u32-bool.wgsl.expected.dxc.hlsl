[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint t = 0u;

uint3 m() {
  t = 1u;
  return uint3((t).xxx);
}

void f() {
  uint3 tint_symbol = m();
  bool3 v = bool3(tint_symbol);
}
