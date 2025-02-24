
static float16_t u = float16_t(1.0h);
void f() {
  bool v = bool(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

