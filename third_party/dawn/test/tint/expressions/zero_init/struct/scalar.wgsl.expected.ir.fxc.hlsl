struct S {
  int i;
  uint u;
  float f;
  bool b;
};


void f() {
  S v = (S)0;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

