SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupBroadcast_2b59c9() {
  int3 arg_0 = (1).xxx;
  int3 res = WaveReadLaneAt(arg_0, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_2b59c9()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_2b59c9()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,14-37): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
