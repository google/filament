
void f() {
  float4x2 v = float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

