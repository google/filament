RWByteAddressBuffer output : register(u0);

struct ComputeInputs {
  uint subgroup_invocation_id;
  uint subgroup_size;
};

void main_inner(ComputeInputs inputs) {
  uint tint_symbol_1 = 0u;
  output.GetDimensions(tint_symbol_1);
  uint tint_symbol_2 = (tint_symbol_1 / 4u);
  output.Store((4u * min(inputs.subgroup_invocation_id, (tint_symbol_2 - 1u))), asuint(inputs.subgroup_size));
}

[numthreads(1, 1, 1)]
void main() {
  ComputeInputs tint_symbol_3 = {WaveGetLaneIndex(), WaveGetLaneCount()};
  main_inner(tint_symbol_3);
  return;
}
