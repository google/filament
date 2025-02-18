[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int arr[2][2] = {{1, 2}, {3, 4}};

void f() {
  int v[2][2] = arr;
}
