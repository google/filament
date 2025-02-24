//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupShuffle_323416() {
  int2 res = WaveReadLaneAt((int(1)).xx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffle_323416()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupShuffle_323416() {
  int2 res = WaveReadLaneAt((int(1)).xx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffle_323416()));
}

