
static bool3 u = (true).xxx;
void f() {
  int3 v = int3(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

