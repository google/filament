SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupXor_83b1f3() {
  int4 tint_tmp = (1).xxxx;
  int4 res = asint(WaveActiveBitXor(asuint(tint_tmp)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupXor_83b1f3()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupXor_83b1f3()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,20-53): error X3004: undeclared identifier 'WaveActiveBitXor'


tint executable returned error: exit status 1
