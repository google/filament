[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float4x4 m = float4x4((1.0f).xxxx, (1.0f).xxxx, (1.0f).xxxx, (1.0f).xxxx);
  float4 v1 = m[0];
  float a = v1[0];
}
