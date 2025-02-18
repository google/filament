
void a() {
  int a_1 = int(1);
  int b = a_1;
  int a_2 = int(1);
  int b_1 = a_2;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

