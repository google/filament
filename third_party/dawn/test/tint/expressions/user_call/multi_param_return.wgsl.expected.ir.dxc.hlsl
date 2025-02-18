
int c(int x, int y, int z) {
  int a = (((int(1) + x) + y) + z);
  a = (a + int(2));
  return a;
}

void b() {
  int b_1 = c(int(2), int(3), int(4));
  int v = c(int(3), int(4), int(5));
  b_1 = (b_1 + v);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

