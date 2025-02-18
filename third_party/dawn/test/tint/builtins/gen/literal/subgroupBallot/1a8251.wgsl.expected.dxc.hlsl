//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupBallot_1a8251() {
  uint4 res = WaveActiveBallot(true);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupBallot_1a8251()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupBallot_1a8251() {
  uint4 res = WaveActiveBallot(true);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBallot_1a8251()));
  return;
}
