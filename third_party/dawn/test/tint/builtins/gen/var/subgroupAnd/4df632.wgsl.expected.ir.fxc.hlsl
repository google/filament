SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupAnd_4df632() {
  uint arg_0 = 1u;
  uint res = WaveActiveBitAnd(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupAnd_4df632());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupAnd_4df632());
}

FXC validation failure:
<scrubbed_path>(5,14-36): error X3004: undeclared identifier 'WaveActiveBitAnd'


tint executable returned error: exit status 1
