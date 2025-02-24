SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupExclusiveMul_87f23e() {
  int3 res = WavePrefixProduct((int(1)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_87f23e()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_87f23e()));
}

FXC validation failure:
<scrubbed_path>(4,14-44): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
