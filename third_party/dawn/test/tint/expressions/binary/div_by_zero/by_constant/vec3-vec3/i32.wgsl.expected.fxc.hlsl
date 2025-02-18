int3 tint_div(int3 lhs, int3 rhs) {
  return (lhs / (((rhs == (0).xxx) | ((lhs == (-2147483648).xxx) & (rhs == (-1).xxx))) ? (1).xxx : rhs));
}

[numthreads(1, 1, 1)]
void f() {
  int3 a = int3(1, 2, 3);
  int3 b = int3(0, 5, 0);
  int3 r = tint_div(a, b);
  return;
}
