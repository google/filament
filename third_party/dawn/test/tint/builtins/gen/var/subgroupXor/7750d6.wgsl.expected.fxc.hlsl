SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupXor_7750d6() {
  uint arg_0 = 1u;
  uint res = WaveActiveBitXor(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupXor_7750d6()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupXor_7750d6()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,14-36): error X3004: undeclared identifier 'WaveActiveBitXor'


tint executable returned error: exit status 1
