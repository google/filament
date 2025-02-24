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
  vector<float16_t, 3> v = vector<float16_t, 3>(tint_symbol);
}
