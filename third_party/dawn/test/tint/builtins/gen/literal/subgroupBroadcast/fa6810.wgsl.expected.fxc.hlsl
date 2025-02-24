SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupBroadcast_fa6810() {
  int2 res = WaveReadLaneAt((1).xx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_fa6810()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_fa6810()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-38): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
