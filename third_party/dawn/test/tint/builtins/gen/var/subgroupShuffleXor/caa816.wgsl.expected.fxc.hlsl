SKIP: INVALID

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

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleXor_caa816()));
  return;
}
FXC validation failure:
<scrubbed_path>(6,39-56): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
