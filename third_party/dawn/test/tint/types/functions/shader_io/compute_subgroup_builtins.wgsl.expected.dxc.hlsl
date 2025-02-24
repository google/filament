RWByteAddressBuffer output : register(u0);

void main_inner(uint subgroup_invocation_id, uint subgroup_size) {
  uint tint_symbol_1 = 0u;
  output.GetDimensions(tint_symbol_1);
  uint tint_symbol_2 = (tint_symbol_1 / 4u);
  output.Store((4u * min(subgroup_invocation_id, (tint_symbol_2 - 1u))), asuint(subgroup_size));
}

[numthreads(1, 1, 1)]
void main() {
  main_inner(WaveGetLaneIndex(), WaveGetLaneCount());
  return;
}
