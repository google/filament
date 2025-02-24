//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupBroadcastFirst_0538e1() {
  float res = WaveReadLaneFirst(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcastFirst_0538e1()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupBroadcastFirst_0538e1() {
  float res = WaveReadLaneFirst(1.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcastFirst_0538e1()));
}

