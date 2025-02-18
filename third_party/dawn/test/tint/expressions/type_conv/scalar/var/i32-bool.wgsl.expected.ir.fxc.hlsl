
static int u = int(1);
void f() {
  bool v = bool(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

