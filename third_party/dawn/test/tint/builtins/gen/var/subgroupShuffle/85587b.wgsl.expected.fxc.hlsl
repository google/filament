SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float4 subgroupShuffle_85587b() {
  float4 arg_0 = (1.0f).xxxx;
  uint arg_1 = 1u;
  float4 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_85587b()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_85587b()));
  return;
}
FXC validation failure:
<scrubbed_path>(6,16-43): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
