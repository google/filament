
void c() {
  int a = int(1);
  a = (a + int(2));
}

void b() {
  c();
  c();
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

