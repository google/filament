[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void c(int x, int y, int z) {
  int a = (((1 + x) + y) + z);
  a = (a + 2);
}

void b() {
  c(1, 2, 3);
  c(4, 5, 6);
}
