[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  int m;
  uint3 n;
};

uint f() {
  S a = (S)0;
  return a.n[2];
}
