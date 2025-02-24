struct ComputeInputs {
  uint subgroup_invocation_id;
  uint subgroup_size;
};


RWByteAddressBuffer output : register(u0);
void main_inner(ComputeInputs inputs) {
  uint v = 0u;
  output.GetDimensions(v);
  output.Store((0u + (min(inputs.subgroup_invocation_id, ((v / 4u) - 1u)) * 4u)), inputs.subgroup_size);
}

[numthreads(1, 1, 1)]
void main() {
  ComputeInputs v_1 = {WaveGetLaneIndex(), WaveGetLaneCount()};
  main_inner(v_1);
}

