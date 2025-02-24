
static uint t = 0u;
uint m() {
  t = 1u;
  return uint(t);
}

void f() {
  int v = int(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

