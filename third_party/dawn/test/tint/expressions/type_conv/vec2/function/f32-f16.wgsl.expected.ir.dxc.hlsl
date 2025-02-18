
static float t = 0.0f;
float2 m() {
  t = 1.0f;
  return float2((t).xx);
}

void f() {
  vector<float16_t, 2> v = vector<float16_t, 2>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

