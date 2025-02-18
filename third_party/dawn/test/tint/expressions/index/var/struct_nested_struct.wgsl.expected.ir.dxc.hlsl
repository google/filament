struct T {
  float o;
  uint p;
};

struct S {
  int m;
  T n;
};


uint f() {
  S a = (S)0;
  return a.n.p;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

