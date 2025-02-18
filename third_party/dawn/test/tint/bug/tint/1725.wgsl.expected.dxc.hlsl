ByteAddressBuffer tint_symbol : register(t0);

struct tint_symbol_8 {
  uint tint_symbol_2 : SV_GroupIndex;
};

void tint_symbol_1_inner(uint tint_symbol_2) {
  uint tint_symbol_10 = 0u;
  tint_symbol.GetDimensions(tint_symbol_10);
  uint tint_symbol_11 = (tint_symbol_10 / 4u);
  int tint_symbol_3 = 0;
  int tint_symbol_4 = 0;
  int tint_symbol_5 = 0;
  uint tint_symbol_6 = tint_symbol.Load((4u * min(tint_symbol_2, (tint_symbol_11 - 1u))));
}

[numthreads(1, 1, 1)]
void tint_symbol_1(tint_symbol_8 tint_symbol_7) {
  tint_symbol_1_inner(tint_symbol_7.tint_symbol_2);
  return;
}
