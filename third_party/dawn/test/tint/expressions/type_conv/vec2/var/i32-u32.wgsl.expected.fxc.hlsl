[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int2 u = (1).xx;

void f() {
  uint2 v = uint2(u);
}
