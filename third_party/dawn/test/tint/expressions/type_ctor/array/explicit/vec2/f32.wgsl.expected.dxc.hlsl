[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float2 arr[2] = {(1.0f).xx, (2.0f).xx};

void f() {
  float2 v[2] = arr;
}
