//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupShuffle_bbb06c() {
  int2 res = WaveReadLaneAt((1).xx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffle_bbb06c()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupShuffle_bbb06c() {
  int2 res = WaveReadLaneAt((1).xx, 1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffle_bbb06c()));
  return;
}
