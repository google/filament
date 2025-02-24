SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupExclusiveMul_25d1b9() {
  float2 res = WavePrefixProduct((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveMul_25d1b9()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveMul_25d1b9()));
}

FXC validation failure:
<scrubbed_path>(4,16-43): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
