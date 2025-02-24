[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float3x4 v = float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
}
