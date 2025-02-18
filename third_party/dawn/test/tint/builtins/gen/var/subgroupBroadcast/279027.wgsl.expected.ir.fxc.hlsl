SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupBroadcast_279027() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupBroadcast_279027());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupBroadcast_279027());
}

FXC validation failure:
<scrubbed_path>(5,15-39): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
