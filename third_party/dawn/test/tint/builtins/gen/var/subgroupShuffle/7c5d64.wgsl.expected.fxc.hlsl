SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupShuffle_7c5d64() {
  float3 arg_0 = (1.0f).xxx;
  int arg_1 = 1;
  float3 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_7c5d64()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_7c5d64()));
  return;
}
FXC validation failure:
<scrubbed_path>(6,16-43): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
