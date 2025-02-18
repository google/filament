[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct A {
  int a;
};
struct B {
  int b;
};

B f(A a) {
  B tint_symbol = (B)0;
  return tint_symbol;
}
