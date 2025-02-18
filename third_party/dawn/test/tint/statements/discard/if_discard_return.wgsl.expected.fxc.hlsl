[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f(bool cond) {
  if (cond) {
    discard;
    return;
  }
}
