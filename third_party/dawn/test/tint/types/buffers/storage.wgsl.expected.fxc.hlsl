ByteAddressBuffer weights : register(t0);

void main() {
  uint tint_symbol_1 = 0u;
  weights.GetDimensions(tint_symbol_1);
  uint tint_symbol_2 = (tint_symbol_1 / 4u);
  float a = asfloat(weights.Load((4u * min(0u, (tint_symbol_2 - 1u)))));
  return;
}
