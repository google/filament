SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupInclusiveAdd_fabbde() {
  int2 res = (WavePrefixSum((int(1)).xx) + (int(1)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveAdd_fabbde()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveAdd_fabbde()));
}

FXC validation failure:
<scrubbed_path>(4,15-40): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
