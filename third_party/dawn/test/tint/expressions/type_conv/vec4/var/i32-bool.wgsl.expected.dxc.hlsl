[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int4 u = (1).xxxx;

void f() {
  bool4 v = bool4(u);
}
