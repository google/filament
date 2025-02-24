
static int t = int(0);
int2 m() {
  t = int(1);
  return int2((t).xx);
}

void f() {
  uint2 v = uint2(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

