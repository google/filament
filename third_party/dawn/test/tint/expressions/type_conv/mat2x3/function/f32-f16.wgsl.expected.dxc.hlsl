[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float t = 0.0f;

float2x3 m() {
  t = (t + 1.0f);
  return float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}

void f() {
  float2x3 tint_symbol = m();
  matrix<float16_t, 2, 3> v = matrix<float16_t, 2, 3>(tint_symbol);
}
