uint3 tint_div(uint3 lhs, uint3 rhs) {
  return (lhs / ((rhs == (0u).xxx) ? (1u).xxx : rhs));
}

[numthreads(1, 1, 1)]
void f() {
  uint3 a = uint3(1u, 2u, 3u);
  uint3 b = uint3(0u, 5u, 0u);
  uint3 r = tint_div(a, b);
  return;
}
