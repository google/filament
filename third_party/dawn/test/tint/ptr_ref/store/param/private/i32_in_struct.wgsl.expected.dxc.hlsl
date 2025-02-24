struct str {
  int i;
};

void func(inout int pointer) {
  pointer = 42;
}

static str P = (str)0;

[numthreads(1, 1, 1)]
void main() {
  func(P.i);
  return;
}
