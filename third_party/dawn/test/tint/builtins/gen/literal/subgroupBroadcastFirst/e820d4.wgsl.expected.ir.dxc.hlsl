//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupBroadcastFirst_e820d4() {
  int3 res = WaveReadLaneFirst((int(1)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcastFirst_e820d4()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupBroadcastFirst_e820d4() {
  int3 res = WaveReadLaneFirst((int(1)).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcastFirst_e820d4()));
}

