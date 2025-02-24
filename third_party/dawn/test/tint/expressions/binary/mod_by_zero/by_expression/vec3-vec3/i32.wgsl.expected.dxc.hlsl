int3 tint_mod(int3 lhs, int3 rhs) {
  int3 rhs_or_one = (((rhs == (0).xxx) | ((lhs == (-2147483648).xxx) & (rhs == (-1).xxx))) ? (1).xxx : rhs);
  if (any(((uint3((lhs | rhs_or_one)) & (2147483648u).xxx) != (0u).xxx))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

[numthreads(1, 1, 1)]
void f() {
  int3 a = int3(1, 2, 3);
  int3 b = int3(0, 5, 0);
  int3 r = tint_mod(a, (b + b));
  return;
}
