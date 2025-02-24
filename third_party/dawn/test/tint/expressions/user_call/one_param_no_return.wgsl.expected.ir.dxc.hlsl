
void c(int z) {
  int a = (int(1) + z);
  a = (a + int(2));
}

void b() {
  c(int(2));
  c(int(3));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

