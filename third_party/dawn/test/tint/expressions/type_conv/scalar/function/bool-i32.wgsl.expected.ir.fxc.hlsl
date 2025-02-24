
static bool t = false;
bool m() {
  t = true;
  return bool(t);
}

void f() {
  int v = int(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

