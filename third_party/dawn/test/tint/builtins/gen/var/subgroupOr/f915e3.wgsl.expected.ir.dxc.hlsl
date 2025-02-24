//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupOr_f915e3() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveActiveBitOr(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupOr_f915e3());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupOr_f915e3() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveActiveBitOr(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupOr_f915e3());
}

