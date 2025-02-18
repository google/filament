SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupShuffleXor_f7b453() {
  float2 arg_0 = (1.0f).xx;
  uint arg_1 = 1u;
  float2 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() ^ arg_1));
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
<scrubbed_path>(6,39-56): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
