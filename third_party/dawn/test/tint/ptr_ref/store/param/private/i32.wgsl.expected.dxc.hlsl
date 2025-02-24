void func(inout int pointer) {
  pointer = 42;
}

static int P = 0;

[numthreads(1, 1, 1)]
void main() {
  func(P);
  return;
}
