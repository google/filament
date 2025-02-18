float tint_degrees(float param_0) {
  return param_0 * 57.29577951308232286465;
}

void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  a = 42.0f;
  b = tint_degrees(a);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
