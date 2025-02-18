
static float2x3 u = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
void f() {
  matrix<float16_t, 2, 3> v = matrix<float16_t, 2, 3>(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

