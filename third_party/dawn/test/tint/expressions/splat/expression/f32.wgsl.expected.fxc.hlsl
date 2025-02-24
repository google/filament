[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float2 v2 = (3.0f).xx;
  float3 v3 = (3.0f).xxx;
  float4 v4 = (3.0f).xxxx;
}
