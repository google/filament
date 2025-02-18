
void f() {
  int i = int(1);
  float2 a = float4x2((0.0f).xx, (0.0f).xx, float2(4.0f, 0.0f), (0.0f).xx)[min(uint(i), 3u)];
  int b = int2(int(0), int(1))[min(uint(i), 1u)];
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

