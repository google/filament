
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / (((rhs == 0u)) ? (1u) : (rhs)));
}

[numthreads(1, 1, 1)]
void f() {
  uint a = 1u;
  uint b = 2u;
  uint r = tint_div_u32(a, b);
}

