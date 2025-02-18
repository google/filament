SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupExclusiveMul_019660() {
  int4 res = WavePrefixProduct((int(1)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveMul_019660()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveMul_019660()));
}

FXC validation failure:
<scrubbed_path>(4,14-45): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
