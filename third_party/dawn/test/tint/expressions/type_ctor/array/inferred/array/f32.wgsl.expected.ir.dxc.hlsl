
static const float v_1[2][2] = {{1.0f, 2.0f}, {3.0f, 4.0f}};
static float arr[2][2] = v_1;
void f() {
  float v[2][2] = arr;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

