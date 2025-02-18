SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float4 subgroupShuffleXor_c88290() {
  float4 res = WaveReadLaneAt((1.0f).xxxx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleXor_c88290()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleXor_c88290()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,45-62): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
