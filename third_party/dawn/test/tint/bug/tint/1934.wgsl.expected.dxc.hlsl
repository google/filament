[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void v() {
  int i = 1;
  int b = (1).xx[min(uint(i), 1u)];
}
