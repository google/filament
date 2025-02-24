void func(int value, inout int pointer) {
  pointer = value;
  return;
}

void main_1() {
  int i = 0;
  i = 123;
  func(123, i);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
