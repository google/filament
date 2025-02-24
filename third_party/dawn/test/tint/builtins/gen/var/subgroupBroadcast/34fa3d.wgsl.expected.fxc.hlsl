SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupBroadcast_34fa3d() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_34fa3d()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_34fa3d()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,15-39): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
