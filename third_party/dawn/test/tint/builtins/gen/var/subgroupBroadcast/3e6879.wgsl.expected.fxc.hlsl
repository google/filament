SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupBroadcast_3e6879() {
  int2 arg_0 = (1).xx;
  int2 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_3e6879()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_3e6879()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,14-38): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
