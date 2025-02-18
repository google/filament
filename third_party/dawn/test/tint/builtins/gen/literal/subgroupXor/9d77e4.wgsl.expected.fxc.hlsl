SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupXor_9d77e4() {
  uint4 res = WaveActiveBitXor((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupXor_9d77e4()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupXor_9d77e4()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-41): error X3004: undeclared identifier 'WaveActiveBitXor'


tint executable returned error: exit status 1
