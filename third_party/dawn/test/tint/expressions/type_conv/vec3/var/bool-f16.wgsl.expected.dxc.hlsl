[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool3 u = (true).xxx;

void f() {
  vector<float16_t, 3> v = vector<float16_t, 3>(u);
}
