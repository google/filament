SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupOr_0bc264() {
  uint res = WaveActiveBitOr(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupOr_0bc264());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupOr_0bc264());
}

FXC validation failure:
<scrubbed_path>(4,14-32): error X3004: undeclared identifier 'WaveActiveBitOr'


tint executable returned error: exit status 1
