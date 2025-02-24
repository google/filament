SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupAnd_4df632() {
  uint res = WaveActiveBitAnd(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAnd_4df632()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAnd_4df632()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-33): error X3004: undeclared identifier 'WaveActiveBitAnd'


tint executable returned error: exit status 1
