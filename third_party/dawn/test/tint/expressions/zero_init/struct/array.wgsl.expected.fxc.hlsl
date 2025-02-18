[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  float a[4];
};

void f() {
  S v = (S)0;
}
