void func(int value, inout int pointer) {
  pointer = value;
}

[numthreads(1, 1, 1)]
void main() {
  int i = 123;
  func(123, i);
  return;
}
