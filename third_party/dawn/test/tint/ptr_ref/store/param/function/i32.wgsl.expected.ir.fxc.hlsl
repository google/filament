
void func(inout int pointer) {
  pointer = int(42);
}

[numthreads(1, 1, 1)]
void main() {
  int F = int(0);
  func(F);
}

