
static int2 u = (int(1)).xx;
void f() {
  uint2 v = uint2(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

