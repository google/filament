
static bool t = false;
bool m() {
  t = true;
  return bool(t);
}

void f() {
  float v = float(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

