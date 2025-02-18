SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupAnd_4adc72() {
  int2 arg_0 = (int(1)).xx;
  int2 res = asint(WaveActiveBitAnd(asuint(arg_0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupAnd_4adc72()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupAnd_4adc72()));
}

FXC validation failure:
<scrubbed_path>(5,20-50): error X3004: undeclared identifier 'WaveActiveBitAnd'


tint executable returned error: exit status 1
