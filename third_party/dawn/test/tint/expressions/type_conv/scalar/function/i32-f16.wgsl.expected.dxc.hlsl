[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int t = 0;

int m() {
  t = 1;
  return int(t);
}

void f() {
  int tint_symbol = m();
  float16_t v = float16_t(tint_symbol);
}
