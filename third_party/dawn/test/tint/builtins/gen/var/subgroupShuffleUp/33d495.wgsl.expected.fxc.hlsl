SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float4 subgroupShuffleUp_33d495() {
  float4 arg_0 = (1.0f).xxxx;
  uint arg_1 = 1u;
  float4 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() - arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleUp_33d495()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleUp_33d495()));
  return;
}
FXC validation failure:
<scrubbed_path>(6,39-56): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
