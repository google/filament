
static int4 u = (int(1)).xxxx;
void f() {
  uint4 v = uint4(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

