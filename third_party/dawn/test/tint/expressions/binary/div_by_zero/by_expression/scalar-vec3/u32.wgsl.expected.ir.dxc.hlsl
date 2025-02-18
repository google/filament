
uint3 tint_div_v3u32(uint3 lhs, uint3 rhs) {
  return (lhs / (((rhs == (0u).xxx)) ? ((1u).xxx) : (rhs)));
}

[numthreads(1, 1, 1)]
void f() {
  uint a = 4u;
  uint3 b = uint3(0u, 2u, 0u);
  uint3 v = (b + b);
  uint3 r = tint_div_v3u32(uint3((a).xxx), v);
}

