[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  {
    for(int i = 0; false; ) {
    }
  }
}
