
float get_f32() {
  return 1.0f;
}

void f() {
  float2 v2 = float2((get_f32()).xx);
  float3 v3 = float3((get_f32()).xxx);
  float4 v4 = float4((get_f32()).xxxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

