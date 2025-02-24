int3 tint_insert_bits(int3 v, int3 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint3((offset).xxx)) : (0).xxx) & int3((int(mask)).xxx)) | (v & int3((int(~(mask))).xxx)));
}

void f_1() {
  int3 v = (0).xxx;
  int3 n = (0).xxx;
  uint offset_1 = 0u;
  uint count = 0u;
  int3 x_16 = tint_insert_bits(v, n, offset_1, count);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
