SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupOr_3f60e0() {
  int2 tint_tmp = (1).xx;
  int2 res = asint(WaveActiveBitOr(asuint(tint_tmp)));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupOr_3f60e0()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupOr_3f60e0()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,20-52): error X3004: undeclared identifier 'WaveActiveBitOr'


tint executable returned error: exit status 1
