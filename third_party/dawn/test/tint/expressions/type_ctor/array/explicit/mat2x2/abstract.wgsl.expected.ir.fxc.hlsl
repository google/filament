
static const float2x2 v_1[2] = {float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f)), float2x2(float2(5.0f, 6.0f), float2(7.0f, 8.0f))};
static float2x2 arr[2] = v_1;
void f() {
  float2x2 v[2] = arr;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

