//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupAnd_ad0cd3() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = WaveActiveBitAnd(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupAnd_ad0cd3());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupAnd_ad0cd3() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = WaveActiveBitAnd(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupAnd_ad0cd3());
}

