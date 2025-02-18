SKIP: INVALID

RWByteAddressBuffer output : register(u0);

struct ComputeInputs {
  uint subgroup_invocation_id;
  uint subgroup_size;
};

void main_inner(ComputeInputs inputs) {
  output.Store((4u * inputs.subgroup_invocation_id), asuint(inputs.subgroup_size));
}

[numthreads(1, 1, 1)]
void main() {
  ComputeInputs tint_symbol = {WaveGetLaneIndex(), WaveGetLaneCount()};
  main_inner(tint_symbol);
  return;
}
FXC validation failure:
<scrubbed_path>(14,32-49): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
