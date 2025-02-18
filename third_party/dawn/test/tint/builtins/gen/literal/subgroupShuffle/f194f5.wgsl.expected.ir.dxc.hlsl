//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupShuffle_f194f5() {
  uint res = WaveReadLaneAt(1u, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupShuffle_f194f5());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupShuffle_f194f5() {
  uint res = WaveReadLaneAt(1u, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupShuffle_f194f5());
}

