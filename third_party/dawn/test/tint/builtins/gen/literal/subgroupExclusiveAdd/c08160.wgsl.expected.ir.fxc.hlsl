SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupExclusiveAdd_c08160() {
  int3 res = WavePrefixSum((int(1)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_c08160()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_c08160()));
}

FXC validation failure:
<scrubbed_path>(4,14-40): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
