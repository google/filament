//
// fragment_main
//
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
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float4 subgroupShuffleUp_33d495() {
  float4 arg_0 = (1.0f).xxxx;
  uint arg_1 = 1u;
  float4 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() - arg_1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleUp_33d495()));
  return;
}
