[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint4 u = (1u).xxxx;

void f() {
  bool4 v = bool4(u);
}
