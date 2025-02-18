SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupExclusiveAdd_ec300f() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WavePrefixSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupExclusiveAdd_ec300f());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupExclusiveAdd_ec300f());
}

FXC validation failure:
<scrubbed_path>(5,15-34): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
