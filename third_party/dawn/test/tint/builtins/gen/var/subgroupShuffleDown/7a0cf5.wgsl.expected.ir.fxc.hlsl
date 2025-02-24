SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupShuffleDown_7a0cf5() {
  float2 arg_0 = (1.0f).xx;
  uint arg_1 = 1u;
  float2 v = arg_0;
  uint v_1 = arg_1;
  float2 res = WaveReadLaneAt(v, (WaveGetLaneIndex() + v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleDown_7a0cf5()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleDown_7a0cf5()));
}

FXC validation failure:
<scrubbed_path>(8,35-52): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
