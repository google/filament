SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupAnd_376802() {
  uint2 res = WaveActiveBitAnd((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupAnd_376802());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupAnd_376802());
}

FXC validation failure:
<scrubbed_path>(4,15-39): error X3004: undeclared identifier 'WaveActiveBitAnd'


tint executable returned error: exit status 1
