[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int f() {
  int i = 0;
  while((i < 4)) {
    i = (i + 1);
  }
  return i;
}
