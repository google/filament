[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct a {
  int a;
};

void f(a a_1) {
  a b = a_1;
}
