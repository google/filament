struct str {
  int i;
};

void func(inout str pointer) {
  str tint_symbol = (str)0;
  pointer = tint_symbol;
}

static str P[4] = (str[4])0;

[numthreads(1, 1, 1)]
void main() {
  func(P[2]);
  return;
}
