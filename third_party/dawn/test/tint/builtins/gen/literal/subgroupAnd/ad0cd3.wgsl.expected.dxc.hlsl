//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupAnd_ad0cd3() {
  uint3 res = WaveActiveBitAnd((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupAnd_ad0cd3()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupAnd_ad0cd3() {
  uint3 res = WaveActiveBitAnd((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAnd_ad0cd3()));
  return;
}
