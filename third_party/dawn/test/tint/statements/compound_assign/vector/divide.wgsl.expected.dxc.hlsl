[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer v : register(u0);

int4 tint_div(int4 lhs, int4 rhs) {
  return (lhs / (((rhs == (0).xxxx) | ((lhs == (-2147483648).xxxx) & (rhs == (-1).xxxx))) ? (1).xxxx : rhs));
}

void foo() {
  v.Store4(0u, asuint(tint_div(asint(v.Load4(0u)), (2).xxxx)));
}
