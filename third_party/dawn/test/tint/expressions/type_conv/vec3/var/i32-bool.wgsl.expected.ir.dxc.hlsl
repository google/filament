
static int3 u = (int(1)).xxx;
void f() {
  bool3 v = bool3(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

