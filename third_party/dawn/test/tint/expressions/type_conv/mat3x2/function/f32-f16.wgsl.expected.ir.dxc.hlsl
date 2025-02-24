
static float t = 0.0f;
float3x2 m() {
  t = (t + 1.0f);
  return float3x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f), float2(5.0f, 6.0f));
}

void f() {
  matrix<float16_t, 3, 2> v = matrix<float16_t, 3, 2>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

