//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupBroadcast_f637f9() {
  int4 res = WaveReadLaneAt((int(1)).xxxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_f637f9()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupBroadcast_f637f9() {
  int4 res = WaveReadLaneAt((int(1)).xxxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_f637f9()));
}

