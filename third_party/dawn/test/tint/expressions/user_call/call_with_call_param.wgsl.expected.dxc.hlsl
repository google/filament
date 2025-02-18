[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

float b(int i) {
  return 2.29999995231628417969f;
}

int c(uint u) {
  return 1;
}

void a() {
  int tint_symbol = c(2u);
  float a_1 = b(tint_symbol);
}
