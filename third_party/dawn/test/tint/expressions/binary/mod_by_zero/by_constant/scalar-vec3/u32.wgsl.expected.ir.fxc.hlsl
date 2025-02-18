
uint3 tint_mod_v3u32(uint3 lhs, uint3 rhs) {
  uint3 v = (((rhs == (0u).xxx)) ? ((1u).xxx) : (rhs));
  return (lhs - ((lhs / v) * v));
}

[numthreads(1, 1, 1)]
void f() {
  uint a = 4u;
  uint3 b = uint3(0u, 2u, 0u);
  uint3 r = tint_mod_v3u32(uint3((a).xxx), b);
}

