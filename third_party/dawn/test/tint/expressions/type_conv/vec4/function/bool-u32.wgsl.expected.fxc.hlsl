[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool t = false;

bool4 m() {
  t = true;
  return bool4((t).xxxx);
}

void f() {
  bool4 tint_symbol = m();
  uint4 v = uint4(tint_symbol);
}
