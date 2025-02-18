
static uint u = 1u;
void f() {
  bool v = bool(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

