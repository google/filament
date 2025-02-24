int3 tint_extract_bits(int3 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  int3 shl_result = ((shl < 32u) ? (v << uint3((shl).xxx)) : (0).xxx);
  return ((shr < 32u) ? (shl_result >> uint3((shr).xxx)) : ((shl_result >> (31u).xxx) >> (1u).xxx));
}

void f_1() {
  int3 v = (0).xxx;
  uint offset_1 = 0u;
  uint count = 0u;
  int3 x_15 = tint_extract_bits(v, offset_1, count);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
