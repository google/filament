SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupShuffle_21f083() {
  uint2 res = WaveReadLaneAt((1u).xx, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupShuffle_21f083());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupShuffle_21f083());
}

FXC validation failure:
<scrubbed_path>(4,15-45): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
