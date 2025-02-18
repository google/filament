struct str {
  int i;
};


void func(inout str pointer) {
  str v = (str)0;
  pointer = v;
}

[numthreads(1, 1, 1)]
void main() {
  str F[4] = (str[4])0;
  func(F[2u]);
}

