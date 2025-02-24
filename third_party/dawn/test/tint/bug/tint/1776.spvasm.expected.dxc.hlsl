struct S {
  float4 a;
  int b;
};

ByteAddressBuffer sb : register(t0);

S sb_load(uint offset) {
  S tint_symbol_3 = {asfloat(sb.Load4((offset + 0u))), asint(sb.Load((offset + 16u)))};
  return tint_symbol_3;
}

void main_1() {
  uint tint_symbol_1 = 0u;
  sb.GetDimensions(tint_symbol_1);
  uint tint_symbol_2 = ((tint_symbol_1 - 0u) / 32u);
  S x_18 = sb_load((32u * min(1u, (tint_symbol_2 - 1u))));
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
