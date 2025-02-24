[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool2 u = (true).xx;

void f() {
  int2 v = int2(u);
}
