//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupBroadcast_6290a2() {
  int4 res = WaveReadLaneAt((int(1)).xxxx, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_6290a2()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupBroadcast_6290a2() {
  int4 res = WaveReadLaneAt((int(1)).xxxx, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_6290a2()));
}

