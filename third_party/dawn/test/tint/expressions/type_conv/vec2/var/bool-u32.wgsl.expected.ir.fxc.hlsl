
static bool2 u = (true).xx;
void f() {
  uint2 v = uint2(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

