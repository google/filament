struct FragmentInputs {
  uint subgroup_invocation_id;
  uint subgroup_size;
};


RWByteAddressBuffer output : register(u0);
void main_inner(FragmentInputs inputs) {
  uint v = 0u;
  output.GetDimensions(v);
  output.Store((0u + (min(inputs.subgroup_invocation_id, ((v / 4u) - 1u)) * 4u)), inputs.subgroup_size);
}

void main() {
  FragmentInputs v_1 = {WaveGetLaneIndex(), WaveGetLaneCount()};
  main_inner(v_1);
}

