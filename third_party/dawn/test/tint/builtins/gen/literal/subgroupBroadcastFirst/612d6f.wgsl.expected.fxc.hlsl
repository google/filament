SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupBroadcastFirst_612d6f() {
  uint4 res = WaveReadLaneFirst((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcastFirst_612d6f()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcastFirst_612d6f()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-42): error X3004: undeclared identifier 'WaveReadLaneFirst'


tint executable returned error: exit status 1
