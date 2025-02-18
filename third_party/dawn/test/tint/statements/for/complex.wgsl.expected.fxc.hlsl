[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void some_loop_body() {
}

void f() {
  int j = 0;
  {
    int i = 0;
    while (true) {
      bool tint_tmp = (i < 5);
      if (tint_tmp) {
        tint_tmp = (j < 10);
      }
      if (!((tint_tmp))) { break; }
      some_loop_body();
      j = (i * 30);
      i = (i + 1);
    }
  }
}
