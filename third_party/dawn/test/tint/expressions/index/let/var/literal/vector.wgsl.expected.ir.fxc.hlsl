
float f() {
  float3 v = float3(1.0f, 2.0f, 3.0f);
  int i = int(1);
  return v[min(uint(i), 2u)];
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

