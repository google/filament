[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int t = 0;

int3 m() {
  t = 1;
  return int3((t).xxx);
}

void f() {
  int3 tint_symbol = m();
  bool3 v = bool3(tint_symbol);
}
