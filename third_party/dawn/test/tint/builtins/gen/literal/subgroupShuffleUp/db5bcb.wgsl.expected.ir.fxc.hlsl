SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupShuffleUp_db5bcb() {
  int2 res = WaveReadLaneAt((int(1)).xx, (WaveGetLaneIndex() - 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleUp_db5bcb()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleUp_db5bcb()));
}

FXC validation failure:
<scrubbed_path>(4,43-60): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
