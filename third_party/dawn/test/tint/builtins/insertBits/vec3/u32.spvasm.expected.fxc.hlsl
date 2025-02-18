uint3 tint_insert_bits(uint3 v, uint3 n, uint offset, uint count) {
  uint e = (offset + count);
  uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint3((offset).xxx)) : (0u).xxx) & uint3((mask).xxx)) | (v & uint3((~(mask)).xxx)));
}

void f_1() {
  uint3 v = (0u).xxx;
  uint3 n = (0u).xxx;
  uint offset_1 = 0u;
  uint count = 0u;
  uint3 x_15 = tint_insert_bits(v, n, offset_1, count);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
