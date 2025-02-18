uint tint_insert_bits(uint v, uint n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << offset) : 0u) & mask) | (v & ~(mask)));
}

void f_1() {
  uint v = 0u;
  uint n = 0u;
  uint offset_1 = 0u;
  uint count = 0u;
  uint x_12 = tint_insert_bits(v, n, offset_1, count);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
