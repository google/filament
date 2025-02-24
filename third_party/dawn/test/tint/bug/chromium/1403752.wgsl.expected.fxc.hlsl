[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void d() {
  int j = 0;
  {
    for(; false; ) {
    }
  }
}
