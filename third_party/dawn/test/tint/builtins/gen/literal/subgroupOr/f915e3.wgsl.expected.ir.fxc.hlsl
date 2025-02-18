SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupOr_f915e3() {
  uint4 res = WaveActiveBitOr((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupOr_f915e3());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupOr_f915e3());
}

FXC validation failure:
<scrubbed_path>(4,15-40): error X3004: undeclared identifier 'WaveActiveBitOr'


tint executable returned error: exit status 1
