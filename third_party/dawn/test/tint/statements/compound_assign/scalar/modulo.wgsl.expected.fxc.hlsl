[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer v : register(u0);

int tint_mod(int lhs, int rhs) {
  int rhs_or_one = (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs);
  if (any(((uint((lhs | rhs_or_one)) & 2147483648u) != 0u))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

void foo() {
  v.Store(0u, asuint(tint_mod(asint(v.Load(0u)), 2)));
}
