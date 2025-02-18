struct str {
  int i;
};


static str P = (str)0;
void func(inout int pointer) {
  pointer = int(42);
}

[numthreads(1, 1, 1)]
void main() {
  func(P.i);
}

