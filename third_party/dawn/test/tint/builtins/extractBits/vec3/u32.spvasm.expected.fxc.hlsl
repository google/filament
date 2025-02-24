uint3 tint_extract_bits(uint3 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  uint3 shl_result = ((shl < 32u) ? (v << uint3((shl).xxx)) : (0u).xxx);
  return ((shr < 32u) ? (shl_result >> uint3((shr).xxx)) : ((shl_result >> (31u).xxx) >> (1u).xxx));
}

void f_1() {
  uint3 v = (0u).xxx;
  uint offset_1 = 0u;
  uint count = 0u;
  uint3 x_14 = tint_extract_bits(v, offset_1, count);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
