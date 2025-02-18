struct B {
  int b;
};

struct A {
  int a;
};


B f(A a) {
  B v = (B)0;
  return v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

