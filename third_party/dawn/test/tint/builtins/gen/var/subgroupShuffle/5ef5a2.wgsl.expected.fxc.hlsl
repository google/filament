SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupShuffle_5ef5a2() {
  uint2 arg_0 = (1u).xx;
  uint arg_1 = 1u;
  uint2 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffle_5ef5a2()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffle_5ef5a2()));
  return;
}
FXC validation failure:
<scrubbed_path>(6,15-42): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
