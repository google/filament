
uint tint_mod_u32(uint lhs, uint rhs) {
  uint v = (((rhs == 0u)) ? (1u) : (rhs));
  return (lhs - ((lhs / v) * v));
}

[numthreads(1, 1, 1)]
void f() {
  uint a = 1u;
  uint b = 0u;
  uint r = tint_mod_u32(a, (b + b));
}

