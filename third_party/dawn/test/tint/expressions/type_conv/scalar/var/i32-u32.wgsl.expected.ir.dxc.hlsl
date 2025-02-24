
static int u = int(1);
void f() {
  uint v = uint(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

