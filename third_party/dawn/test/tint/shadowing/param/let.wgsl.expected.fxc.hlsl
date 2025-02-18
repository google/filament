[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f(int a) {
  {
    int a_1 = a;
    int b = a_1;
  }
}
