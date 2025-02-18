struct S {
  int m;
  uint n;
};


uint f() {
  S a = (S)0;
  return a.n;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

