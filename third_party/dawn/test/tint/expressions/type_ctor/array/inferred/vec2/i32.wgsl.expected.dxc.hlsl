[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int2 arr[2] = {(1).xx, (2).xx};

void f() {
  int2 v[2] = arr;
}
