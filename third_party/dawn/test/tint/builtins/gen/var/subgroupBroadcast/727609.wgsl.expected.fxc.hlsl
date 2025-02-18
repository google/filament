SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupBroadcast_727609() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveReadLaneAt(arg_0, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_727609()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_727609()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,15-38): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
