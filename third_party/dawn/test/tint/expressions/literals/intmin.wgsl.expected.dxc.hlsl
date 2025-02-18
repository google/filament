[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int add_int_min_explicit() {
  int a = -2147483648;
  int b = (a + 1);
  int c = -2147483647;
  return c;
}
