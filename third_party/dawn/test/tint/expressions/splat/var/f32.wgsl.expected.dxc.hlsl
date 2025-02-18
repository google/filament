[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float v = 3.0f;
  float2 v2 = float2((v).xx);
  float3 v3 = float3((v).xxx);
  float4 v4 = float4((v).xxxx);
}
