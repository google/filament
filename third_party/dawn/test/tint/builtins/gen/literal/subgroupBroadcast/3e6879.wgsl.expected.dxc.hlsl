//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupBroadcast_3e6879() {
  int2 res = WaveReadLaneAt((1).xx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_3e6879()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupBroadcast_3e6879() {
  int2 res = WaveReadLaneAt((1).xx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_3e6879()));
  return;
}
