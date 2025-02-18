SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupOr_4d4eb0() {
  int4 arg_0 = (int(1)).xxxx;
  int4 res = asint(WaveActiveBitOr(asuint(arg_0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupOr_4d4eb0()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupOr_4d4eb0()));
}

FXC validation failure:
<scrubbed_path>(5,20-49): error X3004: undeclared identifier 'WaveActiveBitOr'


tint executable returned error: exit status 1
