[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float arr[2][2] = {{1.0f, 2.0f}, {3.0f, 4.0f}};

void f() {
  float v[2][2] = arr;
}
