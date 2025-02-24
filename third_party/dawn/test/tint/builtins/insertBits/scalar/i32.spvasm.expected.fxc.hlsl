int tint_insert_bits(int v, int n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << offset) : 0) & int(mask)) | (v & int(~(mask))));
}

void f_1() {
  int v = 0;
  int n = 0;
  uint offset_1 = 0u;
  uint count = 0u;
  int x_15 = tint_insert_bits(v, n, offset_1, count);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
