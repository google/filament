SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupShuffleXor_f7b453() {
  float2 res = WaveReadLaneAt((1.0f).xx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleXor_f7b453()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleXor_f7b453()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,43-60): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
