int tint_extract_bits(int v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  int shl_result = ((shl < 32u) ? (v << shl) : 0);
  return ((shr < 32u) ? (shl_result >> shr) : ((shl_result >> 31u) >> 1u));
}

void f_1() {
  int v = 0;
  uint offset_1 = 0u;
  uint count = 0u;
  int x_14 = tint_extract_bits(v, offset_1, count);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
