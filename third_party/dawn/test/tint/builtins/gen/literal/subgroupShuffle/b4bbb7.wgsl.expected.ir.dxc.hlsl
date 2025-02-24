//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupShuffle_b4bbb7() {
  int3 res = WaveReadLaneAt((int(1)).xxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_b4bbb7()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupShuffle_b4bbb7() {
  int3 res = WaveReadLaneAt((int(1)).xxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_b4bbb7()));
}

