//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupOr_aa74f7() {
  uint2 res = WaveActiveBitOr((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupOr_aa74f7()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupOr_aa74f7() {
  uint2 res = WaveActiveBitOr((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupOr_aa74f7()));
  return;
}
