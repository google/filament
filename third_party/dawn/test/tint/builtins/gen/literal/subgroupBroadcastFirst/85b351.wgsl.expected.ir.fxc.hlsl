SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupBroadcastFirst_85b351() {
  int2 res = WaveReadLaneFirst((int(1)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_85b351()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_85b351()));
}

FXC validation failure:
<scrubbed_path>(4,14-43): error X3004: undeclared identifier 'WaveReadLaneFirst'


tint executable returned error: exit status 1
