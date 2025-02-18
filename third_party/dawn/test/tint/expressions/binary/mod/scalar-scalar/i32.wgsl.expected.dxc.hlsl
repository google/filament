int tint_mod(int lhs, int rhs) {
  int rhs_or_one = (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs);
  if (any(((uint((lhs | rhs_or_one)) & 2147483648u) != 0u))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

[numthreads(1, 1, 1)]
void f() {
  int a = 1;
  int b = 2;
  int r = tint_mod(a, b);
  return;
}
