SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupBroadcastFirst_85b351() {
  int2 arg_0 = (int(1)).xx;
  int2 res = WaveReadLaneFirst(arg_0);
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
<scrubbed_path>(5,14-37): error X3004: undeclared identifier 'WaveReadLaneFirst'


tint executable returned error: exit status 1
