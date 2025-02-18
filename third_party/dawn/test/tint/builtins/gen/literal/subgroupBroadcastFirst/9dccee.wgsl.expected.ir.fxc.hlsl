SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupBroadcastFirst_9dccee() {
  int4 res = WaveReadLaneFirst((int(1)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcastFirst_9dccee()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcastFirst_9dccee()));
}

FXC validation failure:
<scrubbed_path>(4,14-45): error X3004: undeclared identifier 'WaveReadLaneFirst'


tint executable returned error: exit status 1
