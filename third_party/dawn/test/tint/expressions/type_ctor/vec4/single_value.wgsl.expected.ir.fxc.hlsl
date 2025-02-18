
float4 v() {
  return (0.0f).xxxx;
}

void f() {
  float4 a = (1.0f).xxxx;
  float4 b = float4(a);
  float4 c = float4(v());
  float4 d = float4((a * 2.0f));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

