[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint u = 1u;

void f() {
  float16_t v = float16_t(u);
}
