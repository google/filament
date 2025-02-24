uint tint_extract_bits(uint v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  uint shl_result = ((shl < 32u) ? (v << shl) : 0u);
  return ((shr < 32u) ? (shl_result >> shr) : ((shl_result >> 31u) >> 1u));
}

void f_1() {
  uint v = 0u;
  uint offset_1 = 0u;
  uint count = 0u;
  uint x_11 = tint_extract_bits(v, offset_1, count);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
