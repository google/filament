
static float16_t u = float16_t(1.0h);
void f() {
  float v = float(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

