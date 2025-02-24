
void c(int x, int y, int z) {
  int a = (((int(1) + x) + y) + z);
  a = (a + int(2));
}

void b() {
  c(int(1), int(2), int(3));
  c(int(4), int(5), int(6));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

