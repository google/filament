SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupExclusiveAdd_f0f712() {
  int2 res = WavePrefixSum((int(1)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_f0f712()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_f0f712()));
}

FXC validation failure:
<scrubbed_path>(4,14-39): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
