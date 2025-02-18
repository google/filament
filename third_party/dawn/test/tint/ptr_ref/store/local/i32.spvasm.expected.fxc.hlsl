void main_1() {
  int i = 0;
  i = 123;
  i = 123;
  i = 123;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
