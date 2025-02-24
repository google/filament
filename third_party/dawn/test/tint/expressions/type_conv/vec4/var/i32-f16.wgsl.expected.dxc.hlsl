[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int4 u = (1).xxxx;

void f() {
  vector<float16_t, 4> v = vector<float16_t, 4>(u);
}
