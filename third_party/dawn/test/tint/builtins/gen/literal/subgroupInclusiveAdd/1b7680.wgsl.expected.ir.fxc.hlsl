SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupInclusiveAdd_1b7680() {
  int res = (WavePrefixSum(int(1)) + int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveAdd_1b7680()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveAdd_1b7680()));
}

FXC validation failure:
<scrubbed_path>(4,14-34): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
