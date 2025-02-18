SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupBroadcastFirst_612d6f() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveReadLaneFirst(arg_0);
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
<scrubbed_path>(5,15-38): error X3004: undeclared identifier 'WaveReadLaneFirst'


tint executable returned error: exit status 1
