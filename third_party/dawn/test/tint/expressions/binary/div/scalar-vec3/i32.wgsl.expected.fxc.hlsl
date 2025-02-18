int3 tint_div(int lhs, int3 rhs) {
  int3 l = int3((lhs).xxx);
  return (l / (((rhs == (0).xxx) | ((l == (-2147483648).xxx) & (rhs == (-1).xxx))) ? (1).xxx : rhs));
}

[numthreads(1, 1, 1)]
void f() {
  int a = 4;
  int3 b = int3(1, 2, 3);
  int3 r = tint_div(a, b);
  return;
}
