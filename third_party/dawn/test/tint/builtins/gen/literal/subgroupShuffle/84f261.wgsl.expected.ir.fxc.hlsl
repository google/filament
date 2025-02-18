SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupShuffle_84f261() {
  uint4 res = WaveReadLaneAt((1u).xxxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupShuffle_84f261());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupShuffle_84f261());
}

FXC validation failure:
<scrubbed_path>(4,15-43): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
