
static vector<float16_t, 3> u = (float16_t(1.0h)).xxx;
void f() {
  bool3 v = bool3(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

