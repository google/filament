
static bool u = true;
void f() {
  int v = int(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

