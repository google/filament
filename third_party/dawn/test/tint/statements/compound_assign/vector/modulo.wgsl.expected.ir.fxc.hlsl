
RWByteAddressBuffer v : register(u0);
int4 tint_mod_v4i32(int4 lhs, int4 rhs) {
  int4 v_1 = ((((rhs == (int(0)).xxxx) | ((lhs == (int(-2147483648)).xxxx) & (rhs == (int(-1)).xxxx)))) ? ((int(1)).xxxx) : (rhs));
  return (lhs - ((lhs / v_1) * v_1));
}

void foo() {
  v.Store4(0u, asuint(tint_mod_v4i32(asint(v.Load4(0u)), (int(2)).xxxx)));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

