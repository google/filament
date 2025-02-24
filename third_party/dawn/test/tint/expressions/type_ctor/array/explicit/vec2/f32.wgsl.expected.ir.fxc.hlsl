
static const float2 v_1[2] = {(1.0f).xx, (2.0f).xx};
static float2 arr[2] = v_1;
void f() {
  float2 v[2] = arr;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

