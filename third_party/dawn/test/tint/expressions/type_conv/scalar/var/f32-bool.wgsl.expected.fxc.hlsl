[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float u = 1.0f;

void f() {
  bool v = bool(u);
}
