SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupShuffle_4cbb69() {
  uint3 res = WaveReadLaneAt((1u).xxx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_4cbb69()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_4cbb69()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-41): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
