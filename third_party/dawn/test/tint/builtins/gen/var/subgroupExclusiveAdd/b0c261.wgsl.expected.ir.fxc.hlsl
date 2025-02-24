SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupExclusiveAdd_b0c261() {
  int arg_0 = int(1);
  int res = WavePrefixSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_b0c261()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_b0c261()));
}

FXC validation failure:
<scrubbed_path>(5,13-32): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
