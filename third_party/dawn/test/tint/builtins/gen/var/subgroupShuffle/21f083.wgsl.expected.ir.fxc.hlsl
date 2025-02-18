SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupShuffle_21f083() {
  uint2 arg_0 = (1u).xx;
  int arg_1 = int(1);
  uint2 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupShuffle_21f083());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupShuffle_21f083());
}

FXC validation failure:
<scrubbed_path>(6,15-42): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
