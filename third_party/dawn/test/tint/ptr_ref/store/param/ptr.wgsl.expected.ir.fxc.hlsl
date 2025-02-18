
void func(int value, inout int pointer) {
  pointer = value;
}

[numthreads(1, 1, 1)]
void main() {
  int i = int(123);
  func(int(123), i);
}

