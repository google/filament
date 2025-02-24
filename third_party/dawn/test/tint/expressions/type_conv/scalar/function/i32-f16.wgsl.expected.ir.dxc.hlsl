
static int t = int(0);
int m() {
  t = int(1);
  return int(t);
}

void f() {
  float16_t v = float16_t(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

