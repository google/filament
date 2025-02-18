void main_1() {
  float x_11 = 2.0f;
  float2 x_13 = float2(1.0f, 3.0f);
  float3 x_14 = float3(1.0f, 3.0f, 2.0f);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
