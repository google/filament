SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupExclusiveAdd_406ab4() {
  int4 arg_0 = (int(1)).xxxx;
  int4 res = WavePrefixSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_406ab4()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_406ab4()));
}

FXC validation failure:
<scrubbed_path>(5,14-33): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
