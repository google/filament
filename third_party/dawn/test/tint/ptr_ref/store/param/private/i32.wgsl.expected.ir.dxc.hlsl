
static int P = int(0);
void func(inout int pointer) {
  pointer = int(42);
}

[numthreads(1, 1, 1)]
void main() {
  func(P);
}

