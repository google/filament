//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupShuffle_824702() {
  int3 res = WaveReadLaneAt((int(1)).xxx, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_824702()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupShuffle_824702() {
  int3 res = WaveReadLaneAt((int(1)).xxx, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_824702()));
}

