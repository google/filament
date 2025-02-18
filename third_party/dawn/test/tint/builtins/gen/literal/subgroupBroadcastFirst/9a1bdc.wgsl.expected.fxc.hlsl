SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupBroadcastFirst_9a1bdc() {
  int res = WaveReadLaneFirst(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcastFirst_9a1bdc()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcastFirst_9a1bdc()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,13-32): error X3004: undeclared identifier 'WaveReadLaneFirst'


tint executable returned error: exit status 1
