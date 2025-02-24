[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool t = false;

bool3 m() {
  t = true;
  return bool3((t).xxx);
}

void f() {
  bool3 tint_symbol = m();
  vector<float16_t, 3> v = vector<float16_t, 3>(tint_symbol);
}
