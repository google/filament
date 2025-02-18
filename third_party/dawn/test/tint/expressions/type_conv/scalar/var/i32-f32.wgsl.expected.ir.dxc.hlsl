
static int u = int(1);
void f() {
  float v = float(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

