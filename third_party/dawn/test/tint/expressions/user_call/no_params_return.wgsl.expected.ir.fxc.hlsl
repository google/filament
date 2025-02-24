
int c() {
  int a = int(1);
  a = (a + int(2));
  return a;
}

void b() {
  int b_1 = c();
  int v = c();
  b_1 = (b_1 + v);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

