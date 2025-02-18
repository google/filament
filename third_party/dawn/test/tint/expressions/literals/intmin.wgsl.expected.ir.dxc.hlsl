
int add_int_min_explicit() {
  int a = int(-2147483648);
  int b = (a + int(1));
  int c = int(-2147483647);
  return c;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

