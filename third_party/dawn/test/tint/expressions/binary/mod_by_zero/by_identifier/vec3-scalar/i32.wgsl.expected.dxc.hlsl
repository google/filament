int3 tint_mod(int3 lhs, int rhs) {
  int3 r = int3((rhs).xxx);
  int3 rhs_or_one = (((r == (0).xxx) | ((lhs == (-2147483648).xxx) & (r == (-1).xxx))) ? (1).xxx : r);
  if (any(((uint3((lhs | rhs_or_one)) & (2147483648u).xxx) != (0u).xxx))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

[numthreads(1, 1, 1)]
void f() {
  int3 a = int3(1, 2, 3);
  int b = 0;
  int3 r = tint_mod(a, b);
  return;
}
