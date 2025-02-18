int3 tint_mod(int lhs, int3 rhs) {
  int3 l = int3((lhs).xxx);
  int3 rhs_or_one = (((rhs == (0).xxx) | ((l == (-2147483648).xxx) & (rhs == (-1).xxx))) ? (1).xxx : rhs);
  if (any(((uint3((l | rhs_or_one)) & (2147483648u).xxx) != (0u).xxx))) {
    return (l - ((l / rhs_or_one) * rhs_or_one));
  } else {
    return (l % rhs_or_one);
  }
}

[numthreads(1, 1, 1)]
void f() {
  int a = 4;
  int3 b = int3(1, 2, 3);
  int3 r = tint_mod(a, b);
  return;
}
