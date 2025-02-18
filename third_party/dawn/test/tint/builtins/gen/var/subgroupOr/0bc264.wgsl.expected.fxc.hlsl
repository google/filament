SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupOr_0bc264() {
  uint arg_0 = 1u;
  uint res = WaveActiveBitOr(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupOr_0bc264()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupOr_0bc264()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,14-35): error X3004: undeclared identifier 'WaveActiveBitOr'


tint executable returned error: exit status 1
