
void f() {
  int a = int(0);
  int b = a;
  int a_1 = int(0);
  int b_1 = a_1;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

