void deref() {
  float2x3 a = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3 b = a[0];
  a[0] = float3(1.0f, 2.0f, 3.0f);
}

void no_deref() {
  float2x3 a = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3 b = a[0];
  a[0] = float3(1.0f, 2.0f, 3.0f);
}

[numthreads(1, 1, 1)]
void main() {
  deref();
  no_deref();
  return;
}
