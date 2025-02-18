
static uint t = 0u;
uint4 m() {
  t = 1u;
  return uint4((t).xxxx);
}

void f() {
  int4 v = int4(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

