SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupXor_473de8() {
  int2 arg = (int(1)).xx;
  int2 res = asint(WaveActiveBitXor(asuint(arg)));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupXor_473de8()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupXor_473de8()));
}

FXC validation failure:
<scrubbed_path>(5,20-48): error X3004: undeclared identifier 'WaveActiveBitXor'


tint executable returned error: exit status 1
