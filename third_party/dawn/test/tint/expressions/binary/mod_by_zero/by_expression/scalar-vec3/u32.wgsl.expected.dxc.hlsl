uint3 tint_mod(uint lhs, uint3 rhs) {
  uint3 l = uint3((lhs).xxx);
  return (l % ((rhs == (0u).xxx) ? (1u).xxx : rhs));
}

[numthreads(1, 1, 1)]
void f() {
  uint a = 4u;
  uint3 b = uint3(0u, 2u, 0u);
  uint3 r = tint_mod(a, (b + b));
  return;
}
