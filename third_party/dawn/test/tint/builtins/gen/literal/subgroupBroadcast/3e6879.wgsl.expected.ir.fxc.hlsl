SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupBroadcast_3e6879() {
  int2 res = WaveReadLaneAt((int(1)).xx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_3e6879()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_3e6879()));
}

FXC validation failure:
<scrubbed_path>(4,14-44): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
