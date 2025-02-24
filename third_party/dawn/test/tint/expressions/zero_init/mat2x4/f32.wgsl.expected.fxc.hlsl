[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float2x4 v = float2x4((0.0f).xxxx, (0.0f).xxxx);
}
