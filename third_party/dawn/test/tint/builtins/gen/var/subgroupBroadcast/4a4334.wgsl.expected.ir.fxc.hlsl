SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupBroadcast_4a4334() {
  uint2 arg_0 = (1u).xx;
  uint2 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupBroadcast_4a4334());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupBroadcast_4a4334());
}

FXC validation failure:
<scrubbed_path>(5,15-39): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
