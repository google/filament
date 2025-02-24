//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupAnd_d2c9a6() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveActiveBitAnd(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupAnd_d2c9a6()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupAnd_d2c9a6() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveActiveBitAnd(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupAnd_d2c9a6()));
  return;
}
