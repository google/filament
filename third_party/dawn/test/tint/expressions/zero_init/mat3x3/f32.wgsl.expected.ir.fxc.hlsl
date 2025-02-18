
void f() {
  float3x3 v = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

