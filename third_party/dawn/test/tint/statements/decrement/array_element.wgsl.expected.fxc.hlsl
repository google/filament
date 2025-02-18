[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer a : register(u0);

void main() {
  uint tint_symbol_2 = 0u;
  a.GetDimensions(tint_symbol_2);
  uint tint_symbol_3 = (tint_symbol_2 / 4u);
  a.Store((4u * min(1u, (tint_symbol_3 - 1u))), asuint((a.Load((4u * min(1u, (tint_symbol_3 - 1u)))) - 1u)));
}
