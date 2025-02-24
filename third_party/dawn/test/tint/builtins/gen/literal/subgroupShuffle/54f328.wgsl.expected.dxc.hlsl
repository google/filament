//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupShuffle_54f328() {
  uint res = WaveReadLaneAt(1u, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_54f328()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupShuffle_54f328() {
  uint res = WaveReadLaneAt(1u, 1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_54f328()));
  return;
}
