
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == int(0)) | ((lhs == int(-2147483648)) & (rhs == int(-1))))) ? (int(1)) : (rhs)));
}

[numthreads(1, 1, 1)]
void f() {
  int a = int(1);
  int b = int(0);
  int r = tint_div_i32(a, b);
}

