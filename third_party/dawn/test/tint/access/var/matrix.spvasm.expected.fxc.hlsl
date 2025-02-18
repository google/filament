void main_1() {
  float3x3 m = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  float x_16 = m[1].y;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
