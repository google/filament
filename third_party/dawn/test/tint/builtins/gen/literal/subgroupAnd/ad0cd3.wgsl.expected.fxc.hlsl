SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupAnd_ad0cd3() {
  uint3 res = WaveActiveBitAnd((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupAnd_ad0cd3()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAnd_ad0cd3()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-40): error X3004: undeclared identifier 'WaveActiveBitAnd'


tint executable returned error: exit status 1
