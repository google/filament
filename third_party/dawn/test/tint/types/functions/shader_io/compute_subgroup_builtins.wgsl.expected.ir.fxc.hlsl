SKIP: INVALID


RWByteAddressBuffer output : register(u0);
void main_inner(uint subgroup_invocation_id, uint subgroup_size) {
  output.Store((0u + (uint(subgroup_invocation_id) * 4u)), subgroup_size);
}

[numthreads(1, 1, 1)]
void main() {
  uint v = WaveGetLaneIndex();
  main_inner(v, WaveGetLaneCount());
}

FXC validation failure:
<scrubbed_path>(9,12-29): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
