[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  int i = 0;
  {
    for(; (i < 4); ) {
    }
  }
}
