SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupShuffleXor_7435fe() {
  int3 res = WaveReadLaneAt((1).xxx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleXor_7435fe()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleXor_7435fe()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,39-56): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
