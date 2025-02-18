[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static vector<float16_t, 4> u = (float16_t(1.0h)).xxxx;

void f() {
  bool4 v = bool4(u);
}
