struct S {
  float a[4];
};


void f() {
  S v = (S)0;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

