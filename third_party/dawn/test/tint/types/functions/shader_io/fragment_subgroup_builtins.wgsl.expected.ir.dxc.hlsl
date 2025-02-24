
RWByteAddressBuffer output : register(u0);
void main_inner(uint subgroup_invocation_id, uint subgroup_size) {
  uint v = 0u;
  output.GetDimensions(v);
  output.Store((0u + (min(subgroup_invocation_id, ((v / 4u) - 1u)) * 4u)), subgroup_size);
}

void main() {
  main_inner(WaveGetLaneIndex(), WaveGetLaneCount());
}

