ByteAddressBuffer G : register(t0);

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol_1 = 0u;
  G.GetDimensions(tint_symbol_1);
  uint tint_symbol_2 = ((tint_symbol_1 - 0u) / 4u);
  uint l1 = tint_symbol_2;
  uint l2 = tint_symbol_2;
  return;
}
