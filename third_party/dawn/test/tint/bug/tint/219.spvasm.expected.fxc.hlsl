float x_200(inout float2 x_201) {
  float x_212 = x_201.x;
  return x_212;
}

void main_1() {
  float2 x_11 = float2(0.0f, 0.0f);
  float x_12 = x_200(x_11);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
