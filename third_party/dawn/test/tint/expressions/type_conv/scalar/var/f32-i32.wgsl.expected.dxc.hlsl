[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int tint_ftoi(float v) {
  return ((v <= 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

static float u = 1.0f;

void f() {
  int v = tint_ftoi(u);
}
