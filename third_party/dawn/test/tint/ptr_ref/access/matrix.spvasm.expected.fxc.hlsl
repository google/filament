void main_1() {
  float3x3 m = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  m = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));
  m[1] = (5.0f).xxx;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
