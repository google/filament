[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer v : register(u0);

int tint_div(int lhs, int rhs) {
  return (lhs / (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs));
}

void foo() {
  v.Store(0u, asuint(tint_div(asint(v.Load(0u)), 2)));
}
