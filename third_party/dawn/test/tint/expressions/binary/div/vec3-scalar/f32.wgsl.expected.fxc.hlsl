[numthreads(1, 1, 1)]
void f() {
  float3 a = float3(1.0f, 2.0f, 3.0f);
  float b = 4.0f;
  float3 r = (a / b);
  return;
}
