//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupAnd_376802() {
  uint2 res = WaveActiveBitAnd((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupAnd_376802());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupAnd_376802() {
  uint2 res = WaveActiveBitAnd((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupAnd_376802());
}

