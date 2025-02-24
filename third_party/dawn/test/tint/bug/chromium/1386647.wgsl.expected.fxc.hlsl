uint tint_mod(uint lhs, uint rhs) {
  return (lhs % ((rhs == 0u) ? 1u : rhs));
}

struct tint_symbol_1 {
  uint3 v : SV_DispatchThreadID;
};

void f_inner(uint3 v) {
  uint l = (v.x << (tint_mod(v.y, 1u) & 31u));
}

[numthreads(1, 1, 1)]
void f(tint_symbol_1 tint_symbol) {
  f_inner(tint_symbol.v);
  return;
}
