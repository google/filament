
static int t = int(0);
int3 m() {
  t = int(1);
  return int3((t).xxx);
}

void f() {
  bool3 v = bool3(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

