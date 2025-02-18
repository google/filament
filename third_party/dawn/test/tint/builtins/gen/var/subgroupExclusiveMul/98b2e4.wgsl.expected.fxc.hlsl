SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupExclusiveMul_98b2e4() {
  float arg_0 = 1.0f;
  float res = WavePrefixProduct(arg_0);
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
<scrubbed_path>(5,15-38): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
