SKIP: INVALID

RWByteAddressBuffer output : register(u0);

void main_inner(uint subgroup_invocation_id, uint subgroup_size) {
  output.Store((4u * subgroup_invocation_id), asuint(subgroup_size));
}

void main() {
  main_inner(WaveGetLaneIndex(), WaveGetLaneCount());
  return;
}
FXC validation failure:
<scrubbed_path>(8,14-31): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
