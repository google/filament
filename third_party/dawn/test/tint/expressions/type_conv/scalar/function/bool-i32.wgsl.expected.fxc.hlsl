[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool t = false;

bool m() {
  t = true;
  return bool(t);
}

void f() {
  bool tint_symbol = m();
  int v = int(tint_symbol);
}
