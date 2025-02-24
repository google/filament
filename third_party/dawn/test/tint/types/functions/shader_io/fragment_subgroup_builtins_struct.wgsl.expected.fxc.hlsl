SKIP: INVALID

RWByteAddressBuffer output : register(u0);

struct FragmentInputs {
  uint subgroup_invocation_id;
  uint subgroup_size;
};

void main_inner(FragmentInputs inputs) {
  output.Store((4u * inputs.subgroup_invocation_id), asuint(inputs.subgroup_size));
}

void main() {
  FragmentInputs tint_symbol = {WaveGetLaneIndex(), WaveGetLaneCount()};
  main_inner(tint_symbol);
  return;
}
FXC validation failure:
<scrubbed_path>(13,33-50): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
