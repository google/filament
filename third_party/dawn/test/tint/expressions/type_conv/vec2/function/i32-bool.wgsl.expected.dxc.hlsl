[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int t = 0;

int2 m() {
  t = 1;
  return int2((t).xx);
}

void f() {
  int2 tint_symbol = m();
  bool2 v = bool2(tint_symbol);
}
