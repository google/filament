ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol_3 = 0u;
  tint_symbol_1.GetDimensions(tint_symbol_3);
  uint tint_symbol_4 = (tint_symbol_3 / 4u);
  uint tint_symbol_6 = 0u;
  tint_symbol.GetDimensions(tint_symbol_6);
  uint tint_symbol_7 = (tint_symbol_6 / 4u);
  tint_symbol_1.Store((4u * min(0u, (tint_symbol_4 - 1u))), asuint(asfloat(tint_symbol.Load((4u * min(0u, (tint_symbol_7 - 1u)))))));
  return;
}
