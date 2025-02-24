[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int c() {
  int a = 1;
  a = (a + 2);
  return a;
}

void b() {
  int b_1 = c();
  int tint_symbol = b_1;
  int tint_symbol_1 = c();
  b_1 = (tint_symbol + tint_symbol_1);
}
