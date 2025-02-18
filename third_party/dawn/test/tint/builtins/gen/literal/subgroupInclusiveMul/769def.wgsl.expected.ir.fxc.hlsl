SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupInclusiveMul_769def() {
  int3 res = (WavePrefixProduct((int(1)).xxx) * (int(1)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_769def()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_769def()));
}

FXC validation failure:
<scrubbed_path>(4,15-45): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
