[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint4 u = (1u).xxxx;

void f() {
  vector<float16_t, 4> v = vector<float16_t, 4>(u);
}
