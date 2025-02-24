
static float t = 0.0f;
float3 m() {
  t = 1.0f;
  return float3((t).xxx);
}

void f() {
  vector<float16_t, 3> v = vector<float16_t, 3>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

