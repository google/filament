SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupInclusiveAdd_e18ebb() {
  int4 res = (WavePrefixSum((int(1)).xxxx) + (int(1)).xxxx);
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
<scrubbed_path>(4,15-42): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
