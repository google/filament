
static uint3 u = (1u).xxx;
void f() {
  bool3 v = bool3(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

