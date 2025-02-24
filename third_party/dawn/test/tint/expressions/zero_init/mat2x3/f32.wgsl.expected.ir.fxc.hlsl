
void f() {
  float2x3 v = float2x3((0.0f).xxx, (0.0f).xxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

