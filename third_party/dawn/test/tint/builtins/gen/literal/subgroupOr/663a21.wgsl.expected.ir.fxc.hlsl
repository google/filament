SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupOr_663a21() {
  uint3 res = WaveActiveBitOr((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupOr_663a21());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupOr_663a21());
}

FXC validation failure:
<scrubbed_path>(4,15-39): error X3004: undeclared identifier 'WaveActiveBitOr'


tint executable returned error: exit status 1
