[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint arr[2] = {1u, 2u};

void f() {
  uint v[2] = arr;
}
