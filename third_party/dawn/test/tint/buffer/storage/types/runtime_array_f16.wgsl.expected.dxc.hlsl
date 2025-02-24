ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol_3 = 0u;
  tint_symbol_1.GetDimensions(tint_symbol_3);
  uint tint_symbol_4 = (tint_symbol_3 / 2u);
  uint tint_symbol_6 = 0u;
  tint_symbol.GetDimensions(tint_symbol_6);
  uint tint_symbol_7 = (tint_symbol_6 / 2u);
  tint_symbol_1.Store<float16_t>((2u * min(0u, (tint_symbol_4 - 1u))), tint_symbol.Load<float16_t>((2u * min(0u, (tint_symbol_7 - 1u)))));
  return;
}
