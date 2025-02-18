int tint_div(int lhs, int rhs) {
  return (lhs / (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs));
}

[numthreads(1, 1, 1)]
void f() {
  int a = 1;
  int b = 0;
  int r = tint_div(a, (b + b));
  return;
}
