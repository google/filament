
float3 v() {
  return (0.0f).xxx;
}

void f() {
  float3 a = (1.0f).xxx;
  float3 b = float3(a);
  float3 c = float3(v());
  float3 d = float3((a * 2.0f));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

