[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool u = true;

void f() {
  uint v = uint(u);
}
