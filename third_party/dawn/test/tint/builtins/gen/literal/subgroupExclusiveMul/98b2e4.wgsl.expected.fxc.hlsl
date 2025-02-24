SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupExclusiveMul_98b2e4() {
  float res = WavePrefixProduct(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_98b2e4()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_98b2e4()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-37): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
