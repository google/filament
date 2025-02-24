SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupOr_03343f() {
  int3 tint_tmp = (1).xxx;
  int3 res = asint(WaveActiveBitOr(asuint(tint_tmp)));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupOr_03343f()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupOr_03343f()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,20-52): error X3004: undeclared identifier 'WaveActiveBitOr'


tint executable returned error: exit status 1
