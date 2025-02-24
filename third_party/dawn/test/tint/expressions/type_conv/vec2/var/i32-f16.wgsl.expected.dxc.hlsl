[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int2 u = (1).xx;

void f() {
  vector<float16_t, 2> v = vector<float16_t, 2>(u);
}
