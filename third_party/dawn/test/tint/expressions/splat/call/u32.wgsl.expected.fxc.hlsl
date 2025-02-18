[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

uint get_u32() {
  return 1u;
}

void f() {
  uint tint_symbol = get_u32();
  uint2 v2 = uint2((tint_symbol).xx);
  uint tint_symbol_1 = get_u32();
  uint3 v3 = uint3((tint_symbol_1).xxx);
  uint tint_symbol_2 = get_u32();
  uint4 v4 = uint4((tint_symbol_2).xxxx);
}
