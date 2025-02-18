[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

float f(int x) {
  float3 v = float3(1.0f, 2.0f, 3.0f);
  int i = x;
  return v[min(uint(i), 2u)];
}
