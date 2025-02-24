
void f(bool cond) {
  if (cond) {
    discard;
    return;
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

