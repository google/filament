[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  int i = 1;
  int b = int2(1, 2)[min(uint(i), 1u)];
}
