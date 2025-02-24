[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool t = false;

bool2 m() {
  t = true;
  return bool2((t).xx);
}

void f() {
  bool2 tint_symbol = m();
  float2 v = float2(tint_symbol);
}
