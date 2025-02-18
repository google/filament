
static int t = int(0);
int4 m() {
  t = int(1);
  return int4((t).xxxx);
}

void f() {
  vector<float16_t, 4> v = vector<float16_t, 4>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

