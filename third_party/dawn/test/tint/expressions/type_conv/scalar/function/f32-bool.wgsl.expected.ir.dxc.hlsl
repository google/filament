
static float t = 0.0f;
float m() {
  t = 1.0f;
  return float(t);
}

void f() {
  bool v = bool(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

