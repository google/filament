[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint2 arr[2] = {(1u).xx, (2u).xx};

void f() {
  uint2 v[2] = arr;
}
