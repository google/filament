void main_1() {
  float3 v = (0.0f).xxx;
  float x_14 = v.y;
  float2 x_17 = v.xz;
  float3 x_19 = v.xzy;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
