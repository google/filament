//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupShuffleXor_caa816() {
  float3 arg_0 = (1.0f).xxx;
  uint arg_1 = 1u;
  float3 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() ^ arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleXor_caa816()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupShuffleXor_caa816() {
  float3 arg_0 = (1.0f).xxx;
  uint arg_1 = 1u;
  float3 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() ^ arg_1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleXor_caa816()));
  return;
}
