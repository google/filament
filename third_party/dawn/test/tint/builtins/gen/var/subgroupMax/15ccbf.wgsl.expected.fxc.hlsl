SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupMax_15ccbf() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveActiveMax(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMax_15ccbf()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMax_15ccbf()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,15-34): error X3004: undeclared identifier 'WaveActiveMax'


tint executable returned error: exit status 1
