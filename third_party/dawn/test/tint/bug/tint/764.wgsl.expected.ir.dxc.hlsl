
void f() {
  float4x4 m = float4x4((1.0f).xxxx, (1.0f).xxxx, (1.0f).xxxx, (1.0f).xxxx);
  float4 v1 = m[0u];
  float a = v1.x;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

