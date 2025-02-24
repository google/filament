
float foo() {
  int oob = int(99);
  float b = (0.0f).xxxx[min(uint(oob), 3u)];
  float4 v = (0.0f).xxxx;
  v[min(uint(oob), 3u)] = b;
  return b;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

