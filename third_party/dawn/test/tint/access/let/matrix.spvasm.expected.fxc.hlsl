void main_1() {
  float x_24 = 5.0f;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
