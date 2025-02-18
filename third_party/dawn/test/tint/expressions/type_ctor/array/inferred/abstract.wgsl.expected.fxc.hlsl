[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int arr[2] = {1, 2};

void f() {
  int v[2] = arr;
}
