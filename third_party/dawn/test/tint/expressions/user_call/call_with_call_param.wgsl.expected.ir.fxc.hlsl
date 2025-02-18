
float b(int i) {
  return 2.29999995231628417969f;
}

int c(uint u) {
  return int(1);
}

void a() {
  float a_1 = b(c(2u));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

