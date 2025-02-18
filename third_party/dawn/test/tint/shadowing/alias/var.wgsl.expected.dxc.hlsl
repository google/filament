[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  {
    int a_1 = 0;
    int b = a_1;
  }
  int a_2 = 0;
  int b = a_2;
}
