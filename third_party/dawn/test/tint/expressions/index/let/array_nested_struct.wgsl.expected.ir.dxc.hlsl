struct S {
  int m;
  uint n[4];
};


uint f() {
  S a[2] = (S[2])0;
  return a[1u].n[1u];
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

