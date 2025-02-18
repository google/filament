[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int u = 1;

void f() {
  bool v = bool(u);
}
