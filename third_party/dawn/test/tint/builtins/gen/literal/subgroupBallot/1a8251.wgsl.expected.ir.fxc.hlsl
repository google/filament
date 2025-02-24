SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupBallot_1a8251() {
  uint4 res = WaveActiveBallot(true);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupBallot_1a8251());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupBallot_1a8251());
}

FXC validation failure:
<scrubbed_path>(4,15-36): error X3004: undeclared identifier 'WaveActiveBallot'


tint executable returned error: exit status 1
