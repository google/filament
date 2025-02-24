struct str {
  int i;
};


void func(inout int pointer) {
  pointer = int(42);
}

[numthreads(1, 1, 1)]
void main() {
  str F = (str)0;
  func(F.i);
}

