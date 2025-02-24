SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupInclusiveAdd_e18ebb() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  int4 res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_e18ebb()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_e18ebb()));
}

FXC validation failure:
<scrubbed_path>(6,15-30): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
