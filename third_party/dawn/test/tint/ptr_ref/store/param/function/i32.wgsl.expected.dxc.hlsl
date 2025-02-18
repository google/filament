void func(inout int pointer) {
  pointer = 42;
}

[numthreads(1, 1, 1)]
void main() {
  int F = 0;
  func(F);
  return;
}
