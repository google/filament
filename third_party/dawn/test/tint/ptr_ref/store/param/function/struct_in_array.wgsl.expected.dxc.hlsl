struct str {
  int i;
};

void func(inout str pointer) {
  str tint_symbol = (str)0;
  pointer = tint_symbol;
}

[numthreads(1, 1, 1)]
void main() {
  str F[4] = (str[4])0;
  func(F[2]);
  return;
}
