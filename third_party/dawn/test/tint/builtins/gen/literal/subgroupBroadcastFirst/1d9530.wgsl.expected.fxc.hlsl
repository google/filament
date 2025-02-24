SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupBroadcastFirst_1d9530() {
  uint2 res = WaveReadLaneFirst((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_1d9530()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_1d9530()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-40): error X3004: undeclared identifier 'WaveReadLaneFirst'


tint executable returned error: exit status 1
