SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupMin_8bb8c1() {
  uint2 arg_0 = (1u).xx;
  uint2 res = WaveActiveMin(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupMin_8bb8c1());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupMin_8bb8c1());
}

FXC validation failure:
<scrubbed_path>(5,15-34): error X3004: undeclared identifier 'WaveActiveMin'


tint executable returned error: exit status 1
