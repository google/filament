void main_1() {
  uint x_10 = 0u;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
