
static float t = 0.0f;
float3x3 m() {
  t = (t + 1.0f);
  return float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));
}

void f() {
  matrix<float16_t, 3, 3> v = matrix<float16_t, 3, 3>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

