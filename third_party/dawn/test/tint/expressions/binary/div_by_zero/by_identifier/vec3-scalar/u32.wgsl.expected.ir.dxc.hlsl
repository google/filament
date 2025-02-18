
uint3 tint_div_v3u32(uint3 lhs, uint3 rhs) {
  return (lhs / (((rhs == (0u).xxx)) ? ((1u).xxx) : (rhs)));
}

[numthreads(1, 1, 1)]
void f() {
  uint3 a = uint3(1u, 2u, 3u);
  uint b = 0u;
  uint3 v = a;
  uint3 r = tint_div_v3u32(v, uint3((b).xxx));
}

