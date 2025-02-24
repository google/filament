[numthreads(1, 1, 1)]
void f() {
  float4x2 a = float4x2(float2(-1.0f, -2.0f), float2(-3.0f, -4.0f), float2(-5.0f, -6.0f), float2(-7.0f, -8.0f));
  float2x4 b = float2x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f));
  float2x2 r = mul(b, a);
  return;
}
